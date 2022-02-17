
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

static struct platform_device *vscsi_device;

static int __devinit vscsi_probe(struct device *dev)
{
	dprintk("vSCSI: vscsi_probe: probing...\n");

	/* Process vscsi param */
	if (vscsi) vscsi_common_setup(vscsi);

	/* Init the adapter */
	init_adapter(dev);

	return 0;
}

static __devexit int vscsi_device_remove(struct device *dev)
{
        struct Scsi_Host *shost = dev_get_drvdata(dev);

	dprintk("vSCSI: vscsi_device_remove: removing...\n");
	if (shost) {
		scsi_remove_host(shost);
		scsi_host_put(shost);
	}

        return 0;
}

static struct device_driver vscsi_driver = {
        .name   = "vscsi",
        .bus    = &platform_bus_type,
        .probe  = vscsi_probe,
        .remove = __devexit_p(vscsi_device_remove),
};

static int __init vscsi_init(void)
{
        int err;

#if 0
#ifdef DO_SPI
	dprintk("attaching SPI transport template...\n");
	vscsi_transport_template = spi_attach_transport(&vscsi_transport_functions);
	if (!vscsi_transport_template) return -ENODEV;
#endif

	/* Init devices */
	err = vscsi_device_init();
	if (err) return err;

	dprintk("vSCSI: registering driver...\n");
        err = driver_register(&vscsi_driver);
        if (err) return err;

	dprintk("vSCSI: registering device...\n");
        vscsi_device = platform_device_register_simple("vscsi", -1, NULL, 0);
        if (IS_ERR(vscsi_device)) {
                driver_unregister(&vscsi_driver);
                return PTR_ERR(vscsi_device);
        }
#endif

	panic("done");
        return err;
}

static void __exit vscsi_exit(void)
{
#ifdef DO_SPI
	spi_release_transport(vscsi_transport_template);
#endif
	dprintk("vSCSI: unregistering driver...\n");
	platform_device_unregister(vscsi_device);
	dprintk("vSCSI: unregistering device...\n");
	driver_unregister(&vscsi_driver);
}

#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */

static int __init vscsi_init(void)
{
	printk(">>>>> VSCSI INIT <<<<<<<\n");

	/* Init devices */
	if (vscsi_device_init()) return ENODEV;

	/* Process vscsi param */
	if (vscsi) vscsi_common_setup(vscsi);

	/* Register mod */
	scsi_register_module(MODULE_SCSI_HA, &vscsi_template);
	printk("vscsi_template.present: %d\n", vscsi_template.present);
	if (vscsi_template.present) {
		return 0;
	}

	/* Init failed, unreg */
	printk("unregistering and returning ENODEV...\n");
	scsi_unregister_module(MODULE_SCSI_HA, &vscsi_template);
	return -ENODEV;
}

static void __exit vscsi_exit(void)
{
	printk(">>>>> VSCSI EXIT <<<<<<<\n");
	scsi_unregister_module(MODULE_SCSI_HA, &vscsi_template);
}
#endif

//late_initcall(vscsi_init);
module_init(vscsi_init);
module_exit(vscsi_exit);

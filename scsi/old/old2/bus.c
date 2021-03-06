#if 0
static struct platform_device *vscsi_device;

static int __devinit vscsi_probe(struct device *dev)
{
	printk("vSCSI: vscsi_probe: probing...\n");

#if 0
	/* Process vscsi param */
	if (vscsi) vscsi_common_setup(vscsi);
#endif

	vscsi_add_device(TYPE_DISK,0,"/data/uml/FedoraCore6-AMD64-root_fs");

//	init_adapter(dev);

	return 0;
}

static __devexit int vscsi_device_remove(struct device *dev)
{
        struct Scsi_Host *shost = dev_get_drvdata(dev);

	printk("vSCSI: vscsi_device_remove: removing...\n");
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

	/* Init devices */
	err = vscsi_device_init();
	if (err) return err;

	printk("vSCSI: registering driver...\n");
        err = driver_register(&vscsi_driver);
        if (err) return err;

	printk("vSCSI: registering device...\n");
        vscsi_device = platform_device_register_simple("vscsi", -1, NULL, 0);
        if (IS_ERR(vscsi_device)) {
                driver_unregister(&vscsi_driver);
                return PTR_ERR(vscsi_device);
        }

        return err;
}

static void __exit vscsi_exit(void)
{
	printk("vSCSI: unregistering driver...\n");
	platform_device_unregister(vscsi_device);
	printk("vSCSI: unregistering device...\n");
	driver_unregister(&vscsi_driver);
}

//__initcall(vscsi_init);
//late_initcall(vscsi_init)
//device_initcall(vscsi_init);
module_init(vscsi_init);
module_exit(vscsi_exit);
#endif

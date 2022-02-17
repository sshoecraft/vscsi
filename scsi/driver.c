
static struct platform_device *vscsi_device;

static int vscsi_probe(struct platform_device *dev) {
	dprintk("vSCSI: vscsi_probe: probing...\n");

	/* Process vscsi param */
	if (vscsi) vscsi_common_setup(vscsi);

	/* Init the adapter */
	init_adapter(dev);

	return 0;
}

static int vscsi_device_remove(struct platform_device *dev) {
        struct Scsi_Host *shost = platform_get_drvdata(dev);

	dprintk("vSCSI: vscsi_device_remove: removing...\n");
	if (shost) {
		scsi_remove_host(shost);
		scsi_host_put(shost);
	}

        return 0;
}

static struct platform_driver vscsi_driver = {
	.driver = {
		.name		= "vscsi",
		.owner		= THIS_MODULE,
	},
	.probe		= vscsi_probe,
	.remove		= vscsi_device_remove,
};

static int __init vscsi_init(void) {
        int err;

#ifdef DO_SPI
	dprintk("attaching SPI transport template...\n");
	vscsi_transport_template = spi_attach_transport(&vscsi_transport_functions);
	if (!vscsi_transport_template) return -ENODEV;
#endif

	/* Init devices */
	err = vscsi_device_init();
	if (err) return err;

	dprintk("vSCSI: registering driver...\n");
	err = platform_driver_register(&vscsi_driver);
	if (err) return err;

	dprintk("vSCSI: registering device...\n");
        vscsi_device = platform_device_register_simple("vscsi", -1, NULL, 0);
	if (IS_ERR(vscsi_device)) {
		platform_driver_unregister(&vscsi_driver);
		return PTR_ERR(vscsi_device);
	}

//	panic("done");
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
	platform_driver_unregister(&vscsi_driver);
}

//late_initcall(vscsi_init);
module_init(vscsi_init);
module_exit(vscsi_exit);

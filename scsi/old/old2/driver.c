
static int vscsi_probe(struct device *dev) {
	printk("vSCSI: vscsi_probe...\n");

	init_adapter(dev);
	return 0;
}

static int vscsi_remove(struct device *dev) {
	printk("vSCSI: vscsi_remove...\n");

	return 0;
}

static void vscsi_shutdown(struct device *dev) {
	printk("vSCSI: vscsi_shutdown...\n");
}

static struct device_driver vscsi_device_driver = {
	.name		= "vscsi",
	.bus		= &vscsi_bus_driver,
//	.module		= THIS_MODULE,
//	.mod_name	= KBUILD_MODNAME,
	.probe		= vscsi_probe,
	.remove		= vscsi_remove,
	.shutdown	= vscsi_shutdown,
#if 0
	.suspend	= vscsi_suspend,
	.resume		= vscsi_resume,
#endif
};

static struct device vscsi_device = {
	.bus_id		= "vscsi",
	.bus		= &vscsi_bus_driver,
};

static int __init vscsi_init(void)
{
	struct device_driver *drv;
	int err;

#if 0
        dev = kmalloc(sizeof(*dev), GFP_KERNEL);
        if (!dev){
		printk(KERN_ERR "vSCSI: unable to alloc mem for vscsi device!\n");
		return -ENOMEM;
        }
#endif

	vscsi_device_init();
	if (vscsi) vscsi_common_setup(vscsi);

	err = bus_register(&vscsi_bus_driver);
	if (err) {
		printk("vSCSI: error registering bus driver!\n");
		return err;
	}
	else
		printk("vSCSI: bus driver registered.\n");

	drv = &vscsi_device_driver;
	printk("1: %d\n", (drv->bus->probe && drv->probe));
	printk("2: %d\n", (drv->bus->remove && drv->remove));
	printk("3: %d\n", (drv->bus->shutdown && drv->shutdown));

	err = driver_register(&vscsi_device_driver);
	if (err) {
		printk("vSCSI: error registering device driver!\n");
		return err;
	}
	else
		printk("vSCSI: device driver registered.\n");

	err = device_register(&vscsi_device);
	if (err) {
		printk("vSCSI: error registering device driver!\n");
		return err;
	}
	else
		printk("vSCSI: device registered.\n");

	return 0;
}

static void __exit vscsi_exit(void)
{
//	virtual_bus_exit();

	printk("vSCSI: unregistering driver...\n");
	printk("vSCSI: unregistering device...\n");
}

late_initcall(vscsi_init);
//module_init(vscsi_init);
module_exit(vscsi_exit);

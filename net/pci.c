
static struct pci_device_id vnet_pci_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_VIRT, PCI_DEVICE_ID_VNET) },
	{ 0 }
};

MODULE_DEVICE_TABLE(pci, vnet_pci_ids);

static int __devinit vnet_pci_probe( struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int host_unit;
	unsigned char addr[6];
	struct net_device *dev;
	struct vnet_priv *priv;
	int rc;

	dprintk(KERN_INFO "VNET: probing!\n");

	priv = init_adapter(&pdev->dev);
	pci_set_drvdata(pdev, priv);

//	host_unit = os_net_open();
	host_unit = -1;
	printk("host_unit: %d\n", host_unit);
	if (host_unit < 0) {
		panic("os_net_open");
		return -ENOSYS;
	}

//	unit = units++;

	dev = alloc_etherdev(sizeof(*priv));
	if (dev == NULL) {
		printk(KERN_ERR "VNET: could not allocate memory for device.\n");
		rc = -ENOMEM;
		goto error_out_pdev;
	}
	SET_MODULE_OWNER(dev);
	SET_NETDEV_DEV(dev, &pdev->dev);

	spin_lock_init(&priv->ioctl_lock);
	spin_lock_init(&vnet_irq_lock);

	dev->open = vnet_open;
	dev->stop = vnet_stop;
	dev->hard_start_xmit = vnet_hard_start_xmit;
	dev->ethtool_ops = &vnet_ethtool_ops;
	dev->get_stats = vnet_get_stats;
	dev->do_ioctl = vnet_ioctl;
	dev->irq = VNET_IRQ;

	priv = netdev_priv(dev);
	priv->dev = dev;
	priv->unit = dev->ifindex;
	priv->host_unit = host_unit;
	priv->pdev = pdev;
	pci_set_drvdata(pdev, priv);

	/* Get MAC addr from host */
	vnet_host_get_mac(pdev,addr);
	dprintk("VNET: eth%d: addr: %02x:%02x:%02x:%02x:%02x:%02x\n", dev->ifindex,
		addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
	memcpy(&dev->dev_addr,addr,6);

	priv->mii_if.full_duplex = 1;
	priv->mii_if.phy_id_mask = 0x1f;
	priv->mii_if.reg_num_mask = 0x1f;
	priv->mii_if.dev = dev;
	priv->mii_if.mdio_read = vnet_mdio_read;
	priv->mii_if.mdio_write = vnet_mdio_write;
	priv->mii_if.phy_id = 1;

	rc = register_netdev(dev);
	if (rc) {
		printk(KERN_ERR "VNET: could not register device; rc: %d\n", rc);
		goto error_out_dev;
	}

//	vnet_dev[dev->ifindex] = dev;
	vnet_dev[dev->ifindex] = priv;

	dprintk(KERN_INFO "vnet%d: irq %d, HWAddr %02x:%02x:%02x:%02x:%02x:%02x\n",
		unit, NETWORK_IRQ, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

	return 0;

error_out_dev:
	free_netdev(dev);

error_out_pdev:
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);

	return rc;
}

static void __devexit vnet_pci_remove(struct pci_dev *pdev)
{
	struct vnet_priv *priv = dev_get_drvdata(&pdev->dev);
//	struct net_device *net_dev = vnet_dev[priv->unit];

//	unregister_netdev(net_dev);
//	free_netdev(net_dev);
	unregister_netdev(priv->dev);
	free_netdev(priv->dev);
	dev_set_drvdata(&pdev->dev, NULL);
}

static struct pci_driver vnet_pci_driver = {
	.name           = DRV_NAME,
	.id_table       = vnet_pci_ids,
	.probe          = vnet_pci_probe,
	.remove         = __devexit_p(vnet_pci_remove),
};

static int __init vnet_pci_init(void)
{
	int unit, rc;

	dprintk(KERN_INFO "VNET: Initializing...\n");

	/* Init our units */
	for (unit=0; unit < VNET_NUM_UNITS; unit++)
		vnet_dev[unit] = NULL;

	printk(KERN_INFO "VNET: registering...\n");
	rc = pci_register_driver(&vnet_pci_driver);
	if (rc != 0) return rc;

	return vpci_add_device(0, PCI_DEVICE_ID_VNET, PCI_CLASS_NETWORK_ETHERNET, 0);
}

static void __exit vnet_pci_exit(void)
{
	dprintk("VNET: exiting...\n");
        pci_unregister_driver(&vnet_pci_driver);
}

module_init(vnet_pci_init);
module_exit(vnet_pci_exit);

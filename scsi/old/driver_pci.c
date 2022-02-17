
#define VSCSI_DEBUG_PCI 1

static int vscsi_pci_probe( struct pci_dev *pdev, const struct pci_device_id *ent )
{
	struct Scsi_Host *shost;

	if (vscsi) vscsi_common_setup(vscsi);

	shost = init_adapter(&pdev->dev);
	pci_set_drvdata(pdev, shost);
	return 0;
}

/*
 * PCI Remove - hotplug removal
*/
static void __devexit vscsi_pci_remove(struct pci_dev *pdev)
{
	pci_set_drvdata(pdev, NULL);
}

static struct pci_device_id vscsi_pci_ids[] __devinitdata = {
	{ PCI_DEVICE(PCI_VENDOR_ID_VIRT, PCI_DEVICE_ID_VSCSI) },
	{ 0 }
};

static struct pci_driver vscsi_pci_driver = {
	.name		= "vscsi",
	.id_table	= vscsi_pci_ids,
	.probe		= vscsi_pci_probe,
	.remove		= __devexit_p(vscsi_pci_remove),
};

/*
 * PCI Init - module load
*/
static int __init vscsi_pci_init(void) {
	int rc;

	if (vscsi_device_init()) return -ENOSYS;
#if 0
	rc = request_irq(SCSI_IRQ, &vscsi_isr, IRQF_SAMPLE_RANDOM, "vscsi", NULL);
	if (rc) {
		printk(KERN_ERR "vscsi_pci_init: unable to get irq %d", SCSI_IRQ);
		return rc;
	}
	spin_lock_init(&vscsi_isr_lock);
#endif


	printk(KERN_INFO "VSCSI: registering PCI driver\n");
	rc = pci_register_driver(&vscsi_pci_driver);

	/* Add our device to vpci */
	printk(KERN_INFO "VSCSI: registering VPCI device\n");
	vpci_add_device(0, PCI_DEVICE_ID_VSCSI, PCI_CLASS_STORAGE_SCSI, 0);

	return rc;
}

/*
 * PCI Exit - module unload
*/
static void __exit vscsi_pci_exit(void) {
#if VSCSI_DEBUG_PCI
	printk(KERN_INFO "VSCSI: unregistering PCI driver\n");
#endif
        pci_unregister_driver(&vscsi_pci_driver);
}

module_init(vscsi_pci_init);
module_exit(vscsi_pci_exit);

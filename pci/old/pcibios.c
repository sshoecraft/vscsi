
#include <linux/init.h>
#include <linux/pci.h>

extern int pcibios_scanned;
extern int pcibios_last_bus;
extern struct pci_bus *pci_root_bus;
extern struct pci_ops pci_root_ops;

/*
 * Discover remaining PCI buses in case there are peer host bridges.
 * We use the number of last PCI bus provided by the PCI BIOS.
 */
#if 0
static void __devinit pcibios_fixup_peer_bridges(void)
{
	int n, devfn;

	if (pcibios_last_bus <= 0 || pcibios_last_bus >= 0xff)
		return;
	DBG("PCI: Peer bridge fixup\n");

	for (n=0; n <= pcibios_last_bus; n++) {
		u32 l;
		if (pci_find_bus(0, n))
			continue;
		for (devfn = 0; devfn < 256; devfn += 8) {
			if (!raw_pci_ops->read(0, n, devfn, PCI_VENDOR_ID, 2, &l) &&
			    l != 0x0000 && l != 0xffff) {
				DBG("Found device at %02x:%02x [%04x]\n", n, devfn, l);
				printk(KERN_INFO "PCI: Discovered peer bus %02x\n", n);
				pci_scan_bus(n, &pci_root_ops, NULL);
				break;
			}
		}
	}
}
#endif

static int __init pci_legacy_init(void)
{
	if (!raw_pci_ops) {
		printk("PCI: System does not support PCI\n");
		return 0;
	}

	if (pcibios_scanned++)
		return 0;

	printk("PCI: Probing PCI hardware\n");
	pci_root_bus = pcibios_scan_root(0);
	if (pci_root_bus)
		pci_bus_add_devices(pci_root_bus);

	pcibios_fixup_peer_bridges();

	return 0;
}

//subsys_initcall(pci_legacy_init);

/* pcibios dummy funcs */
void pcibios_align_resource(void *a, struct resource *b, resource_size_t c, resource_size_t d) {
	printk("***pcibios_align_resource\n");
}
void pcibios_update_irq(struct pci_dev *d, int irq) {
	printk("***pcibios_update_irq\n");
}
int pcibios_enable_resources(struct pci_dev *dev, int mask) {
	printk("***pcibios_enable_resources\n");
	return -2;
}
void pcibios_disable_resources(struct pci_dev *dev) {
	printk("***pcibios_disable_resources\n");
}
static int vpci_enable_irq(struct pci_dev *pdev) {
	printk("***vpci_enable_irq\n");
	return 0;
}
static void vpci_disable_irq(struct pci_dev *pdev) {
	printk("***vpci_disable_irq\n");
	return;
}
int (*pcibios_enable_irq)(struct pci_dev *dev) = vpci_enable_irq;
void (*pcibios_disable_irq)(struct pci_dev *dev) = vpci_disable_irq;


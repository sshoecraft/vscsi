
#include <linux/init.h>
#include <linux/pci.h>

unsigned int pcibios_irq_mask = 0xfff8;
unsigned long pci_mem_start = 0x10000000;

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
#if 0
void __devinit pcibios_fixup_bus(struct pci_bus *b)
{
	printk("***pcibios_fixup_bus\n");
}
int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	return 0;
}
int pcibios_disable_device(struct pci_dev *dev)
{
	return 0;
}
char * __devinit  pcibios_setup(char *str)
{
	return str;
}
#endif

static int dummy_enable_irq(struct pci_dev *pdev) {
	printk("***dummy_enable_irq\n");
	return 0;
}
static void dummy_disable_irq(struct pci_dev *pdev) {
	printk("***dummy_disable_irq\n");
	return;
}
int (*pcibios_enable_irq)(struct pci_dev *dev) = dummy_enable_irq;
void (*pcibios_disable_irq)(struct pci_dev *dev) = dummy_disable_irq;


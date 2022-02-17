
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
#include <asm-generic/pci_iomap.h>
#endif

unsigned int pcibios_irq_mask = 0xfff8;
unsigned long pci_mem_start = 0x10000000;

#ifndef __devinit
#define __devinit
#endif
#ifndef __iomem
#define __iomem
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
int pci_set_dma_max_seg_size(struct pci_dev *dev, unsigned int size) { return -EIO; }
int pci_set_dma_seg_boundary(struct pci_dev *dev, unsigned long mask) { return -EIO; }

void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long maxlen)
{
	return 0;
}
#endif
void __iomem *pci_ioremap_bar(struct pci_dev *pdev, int bar) { return 0; }

/* pcibios dummy funcs */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,33)
resource_size_t pcibios_align_resource(void *a, const struct resource *b, resource_size_t c, resource_size_t d) {
	return 0;
}
#else
void pcibios_align_resource(void *a, struct resource *b, resource_size_t c, resource_size_t d) {
	printk("***pcibios_align_resource\n");
}
#endif
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
void __devinit pcibios_fixup_bus(struct pci_bus *b)
{
	printk("***pcibios_fixup_bus\n");
}
int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	return 0;
}
void pcibios_disable_device(struct pci_dev *dev) {
	return;
}
char * __devinit  pcibios_setup(char *str)
{
	return str;
}

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

#define PIO_OFFSET      0x10000UL
#define PIO_MASK        0x0ffffUL
#define PIO_RESERVED    0x40000UL

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,18,0)
/* Create a virtual mapping cookie for an IO port range */
void __iomem *ioport_map(unsigned long port, unsigned int nr)
{
        if (port > PIO_MASK)
                return NULL;
        return (void __iomem *) (unsigned long) (port + PIO_OFFSET);
}
#endif
void ioport_unmap(void __iomem *addr)
{
        /* Nothing to do */
}
//EXPORT_SYMBOL(ioport_map);
//EXPORT_SYMBOL(ioport_unmap);

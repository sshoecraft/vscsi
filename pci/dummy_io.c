
#include <linux/version.h>
#include <linux/pci.h>
#include "vpci.h"

/*
 * Dummy I/O funcs for UML
*/
#if 0
unsigned char readb(const volatile void __iomem *addr) {
	printk("lol @ readb attempt\n");
	return 0;
}

unsigned short readw(const volatile void __iomem *addr) {
	printk("lol @ readw attempt\n");
	return 0;
}

unsigned long readl(const volatile void __iomem *addr) {
	printk("lol @ readl attempt\n");
	return 0;
}

unsigned short inw(void *addr) {
	printk("lol @ inw attempt\n");
	return 0;
}

unsigned short outw(void *addr) {
	printk("lol @ outw attempt\n");
	return 0;
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35) && LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
void __iomem * ioremap(unsigned long offset, unsigned long size) {
	printk("***ioremap: offset: %lx, size: %ld\n", offset, size);
	return 0;
}

void __iomem *ioremap_nocache(unsigned long phys_addr, unsigned long size) {
	printk("***ioremap_nocache: phys_addr: %lx, size: %ld\n", phys_addr, size);
	return 0;
}

void iounmap(volatile void __iomem *addr) { 
	printk("***iounmap: addr: %p\n", addr);
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,18,0)
void __iomem *ioport_map(unsigned long port, unsigned int nr)
{
        return (void __iomem *) port;
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35) && LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
void memcpy_fromio(void *dst, const volatile void __iomem *src, long n) {
	printk("***memcpy_fromio: dst: %p, src: %p, n: %ld\n", dst, src, n);
}

int pci_mmap_page_range(struct pci_dev *dev, struct vm_area_struct *vma,
                        enum pci_mmap_state mmap_state, int write_combine)
{
	printk("***pci_mmap_page_range called\n");
	return -EINVAL;
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,18,0)
void devm_ioremap_release(struct device *dev, void *res) { iounmap(*(void __iomem **)res); }
int pcim_iomap_regions(struct pci_dev *pdev, int mask, const char *name) { return -EINVAL; }
void __iomem * const *pcim_iomap_table(struct pci_dev *pdev) { return NULL; }
void pcim_iounmap_regions(struct pci_dev *pdev, int mask) { }
#if 0
void __iomem *pci_ioremap_bar(struct pci_dev *pdev, int bar)
{
        struct resource *res = &pdev->resource[bar];

        /*
         * Make sure the BAR is actually a memory resource, not an IO resource
         */
        if (res->flags & IORESOURCE_UNSET || !(res->flags & IORESOURCE_MEM)) {
                pci_warn(pdev, "can't ioremap BAR %d: %pR\n", bar, res);
                return NULL;
        }
        return ioremap_nocache(res->start, resource_size(res));
}
#endif
#endif

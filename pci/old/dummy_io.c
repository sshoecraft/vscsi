
#include <linux/pci.h>
#include "pci.h"

/*
 * Dummy I/O funcs for UML
*/
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

void memcpy_fromio(void *dst, const volatile void __iomem *src, long n) {
	printk("***memcpy_fromio: dst: %p, src: %p, n: %ld\n", dst, src, n);
}

int pci_mmap_page_range(struct pci_dev *dev, struct vm_area_struct *vma,
                        enum pci_mmap_state mmap_state, int write_combine)
{
	printk("***pci_mmap_page_range called\n");
	return -EINVAL;
}

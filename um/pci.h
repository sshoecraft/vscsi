#ifndef __UM_PCI_H
#define __UM_PCI_H

#include <linux/version.h>
#include <asm-generic/pci.h>
#include <linux/dma-mapping.h>

/* Dummy pci.h for UML (all versions) */

#define pcibios_assign_all_busses()     0
#define pcibios_scan_all_fns(a, b)      0

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,32)
static inline void pcibios_set_master(struct pci_dev *dev) {
	printk("***pcibios_set_master\n");
}
#endif
#ifndef PCI_DMA_BUS_IS_PHYS
#define PCI_DMA_BUS_IS_PHYS     (0)
#endif
#define pci_dma_supported(dev, mask)	0

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,18,0))
static inline void __iomem *ioremap(phys_addr_t offset, unsigned long size)
{
        return (void __iomem*) (unsigned long)offset;
}
static inline void iounmap(void *addr) { }
#ifndef ioremap_nocache
#define ioremap_nocache ioremap
#endif
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)) && (LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0))
static inline void pci_iounmap(struct pci_dev *dev, void __iomem *p) { }
static inline u16 __raw_inw(const volatile void __iomem *addr)
{
        return *(const volatile u16 __force *) addr;
}
struct pci_dev;
extern void __iomem *pci_iomap(struct pci_dev *dev, int bar, unsigned long max);
extern void pci_iounmap(struct pci_dev *dev, void __iomem *);
static inline u8 readb(const volatile void __iomem *addr)
{
        return *(const volatile u8 *) addr;
}

static inline u16 readw(const volatile void __iomem *addr)
{
        return *(const volatile u16 *) addr;
}

static inline u32 readl(const volatile void __iomem *addr)
{
        return *(const volatile u32 *) addr;
}
static inline u16 inw(unsigned long addr)
{
        return readw((volatile void __iomem *) addr);
}

static inline u32 inl(unsigned long addr)
{
        return readl((volatile void __iomem *) addr);
}

#if 0
static inline void outb(u8 b, unsigned long addr)
{
        return writeb(b, (volatile void __iomem *) addr);
}
#endif

static inline void outw(u16 b, unsigned long addr)
{
        return writew(b, (volatile void __iomem *) addr);
}
#define memcpy_fromio(a,b,c)    memcpy((a),(void *)(b),(c))
#endif

#if 0
#ifdef CONFIG_PCI
extern unsigned int pcibios_assign_all_busses(void);
#else
#define pcibios_assign_all_busses()     0
#endif
#endif


#define HAVE_ARCH_PCI_SET_DMA_MAX_SEGMENT_SIZE 1
#define HAVE_ARCH_PCI_SET_DMA_SEGMENT_BOUNDARY 1

extern unsigned long pci_mem_start;
#define PCIBIOS_MIN_IO          0x1000
#define PCIBIOS_MIN_MEM         (pci_mem_start)

#define PCIBIOS_MIN_CARDBUS_IO  0x4000

static inline void acpi_noirq_set(void) {
	printk("***acpi_noirq_set\n");
}
//static inline void pcibios_add_platform_entries(struct pci_dev *dev) {
//	printk("***pcibios_add_platform_entries\n");
//}

/* generic pci stuff */
//#include <asm-generic/pci.h>
//#include <linux/dma-mapping.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,0,0) 
extern int isa_dma_bridge_buggy;
void __iomem *ioport_map(unsigned long port, unsigned int nr);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,18,0)
#define ioremap_wc ioremap_nocache
#else
#define pfn_to_kaddr(pfn)        pfn_to_virt(pfn)
#define pfn_to_virt(pfn)   __va((pfn) << PAGE_SHIFT)
#endif
#endif

#endif

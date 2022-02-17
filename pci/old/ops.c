
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/pci_steve.h>
#include "pci.h"

#define VPCI_DEBUG 0
#define VPCI_DEBUG_IO 0

unsigned long pci_mem_start = 0x10000000;
EXPORT_SYMBOL(pci_mem_start);
unsigned int pcibios_irq_mask = 0xfff8;

struct device_list {
	int bus;
	int device;
	int func;
	unsigned char regs[256];
	struct device_list *next;
};

static struct device_list *devices = 0, *last_device;

#define pci_byte(r,l) *((unsigned char *)(&r[l]))
#define pci_short(r,l) *((unsigned short *)(&r[l]))
#define pci_long(r,l) *((unsigned long *)(&r[l]))

static int vpci_read(unsigned int seg, unsigned int bus, unsigned int devfn, int reg, int len, u32 *value)
{
	int device, func;
	struct device_list *dp;

	/* Linux has encoded the device & func; split them */
	device = devfn >> 3;
	func = devfn & 7;

	if (reg + len > 255) {
		*value = -1;
		return -EINVAL;
	}

	spin_lock(&pci_config_lock);

	*value = 0;
	for(dp = devices; dp; dp = dp->next) {
		if (bus == dp->bus && device == dp->device && func == dp->func) {
#if VPCI_DEBUG_IO
			printk(KERN_INFO "vpci_read: bus: %02x, devfn: %02x "
				"(device: %02x, func: %02x), reg: %02x, len: %d\n", bus, devfn, device, func, reg, len);
#endif
			switch(len) {
			case 1:
				*value = pci_byte(dp->regs, reg);
				break;
			case 2:
				*value = pci_short(dp->regs, reg);
				break;
			case 4:
				*value = pci_long(dp->regs, reg);
				break;
			}
#if VPCI_DEBUG_IO
			printk(KERN_INFO "vpci_read: value: 0x%08x\n", *value);
#endif
		}
	}

	spin_unlock(&pci_config_lock);

	return 0;
}

static int vpci_write(unsigned int seg, unsigned int bus, unsigned int devfn, int reg, int len, u32 value) {
	struct device_list *dp;
	int rc, device, func;

	device = devfn >> 3;
	func = devfn & 7;

	if (reg + len > 255) return -EINVAL;

	spin_lock(&pci_config_lock);

	rc = -EPERM;
	for(dp = devices; dp; dp = dp->next) {
		if (bus == dp->bus && device == dp->device && func == dp->func) {
#if VPCI_DEBUG_IO
			printk(KERN_INFO "vpci_read: bus: %02x, devfn: %02x "
				"(device: %02x, func: %02x), reg: %02x, len: %d, value: %08X\n",
				bus, devfn, device, func, reg, len, value);
#endif
			switch(len) {
			case 1:
//				pci_byte(dp->regs, reg) = *value;
				break;
			case 2:
//				pci_short(dp->regs, reg) = *value;
				break;
			case 4:
//				pci_long(dp->regs, reg) = value;
				break;
			}
#if VPCI_DEBUG_IO
			printk(KERN_INFO "vpci_read: value: 0x%08x\n", value);
#endif
		}
	}

	spin_unlock(&pci_config_lock);

	return rc;
}

struct pci_raw_ops vpci_ops = {
	.read =         vpci_read,
	.write =        vpci_write,
};

static int pci_add_device(int b, int d, int f, int id, int class, int irq) {
	struct device_list *newdev;

#if VPCI_DEBUG
	printk("VPCI: add_new: d: %d, f: %d, id: %x, class: %x, irq: %d\n", d, f, vend, id, class, irq);
#endif

	newdev = kzalloc(sizeof(struct device_list), GFP_KERNEL);
	if (!newdev) {
		printk(KERN_ERR "VPCI: no memory for device info!\n");
		return -ENOMEM;
	}
	memset(newdev, 0, sizeof(*newdev));
	newdev->bus = b;
	newdev->device = d;
	newdev->func = f;
	pci_short(newdev->regs, PCI_VENDOR_ID) = PCI_VENDOR_ID_STEVE;
	pci_short(newdev->regs, PCI_DEVICE_ID) = id;
	pci_short(newdev->regs, PCI_COMMAND) = PCI_COMMAND_FAST_BACK;
	pci_short(newdev->regs, PCI_STATUS) = (PCI_STATUS_FAST_BACK | PCI_STATUS_DEVSEL_FAST);
	pci_short(newdev->regs, PCI_HEADER_TYPE) = 0x80;
	pci_short(newdev->regs, PCI_CLASS_DEVICE) = class;
	pci_byte(newdev->regs, PCI_INTERRUPT_LINE) = irq;
	pci_byte(newdev->regs, PCI_INTERRUPT_PIN) = 1;
	if (devices) {
		last_device->next = newdev;
		last_device = newdev;
	} else {
		devices = newdev;
		last_device = newdev;
	}

	return 0;
}

int vpci_init(void) {
#if VPCI_DEBUG
	printk(KERN_INFO "VPCI: Initializing\n");
#endif

#ifdef CONFIG_COOPERATIVE
	vpci_co_init();
#endif

#if 0
        /* bus,dev,func,id,class,irq */
        add_new(0, 0, 0, PCI_DEVICE_ID_VSCSI, PCI_CLASS_STORAGE_SCSI, 0);
        add_new(0, 1, 0, PCI_DEVICE_ID_VIDE, PCI_CLASS_STORAGE_IDE, 0);
        add_new(0, 2, 0, PCI_DEVICE_ID_VNET, PCI_CLASS_NETWORK_ETHERNET, 0);
        add_new(0, 3, 0, PCI_DEVICE_ID_VVID, PCI_CLASS_DISPLAY_OTHER, 0);
        add_new(0, 4, 0, PCI_DEVICE_ID_VSND, PCI_CLASS_MULTIMEDIA_AUDIO, 0);
#endif

	return 0;
}
arch_initcall(vpci_init);

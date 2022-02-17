
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/version.h>
#include "vpci.h"
//#include "pci.h"

#define VPCI_VERSION "1.00"

MODULE_AUTHOR("Steve Shoecraft <sshoecraft@earthlink.net>");
MODULE_DESCRIPTION("Virtual PCI Bus Driver " VPCI_VERSION);
MODULE_LICENSE("GPL");

#define VPCI_DEBUG 0
#define VPCI_DEBUG_IO 0

DEFINE_SPINLOCK(vpci_config_lock);

struct device_list {
	int bus;
	int device;
	int func;
	unsigned char regs[256];
	struct device_list *next;
};

static struct device_list *devices = 0, *last_device;
static struct pci_bus *vpci_bus;

//int isa_dma_bridge_buggy = 0;
//unsigned long pci_mem_start = 0x10000000;

#define pci_byte(r,l) *((unsigned char *)(&r[l]))
#define pci_short(r,l) *((unsigned short *)(&r[l]))
#define pci_long(r,l) *((unsigned long *)(&r[l]))

#if VPCI_DEBUG
#define dprintk(format, args...) printk("%s(%d): " format ,__FUNCTION__,__LINE__, ## args)
#else
#define dprintk(format, args...) /* noop */
#endif

#define BUSNUM bus->number
static int vpci_read(struct pci_bus *bus, unsigned int devfn, int reg, int len, u32 *value)
{
	int device, func;
	struct device_list *dp;

//	printk("*** vpci_read: bus: %p, devfn: %d, reg: %d, len: %d, value: %p\n", bus, devfn, reg, len, value);

	/* Linux has encoded the device & func; split them */
	device = devfn >> 3;
	func = devfn & 7;

	if (reg + len > 255) {
		*value = -1;
		return -EINVAL;
	}

	spin_lock(&vpci_config_lock);

	*value = 0;
	for(dp = devices; dp; dp = dp->next) {
		if (BUSNUM == dp->bus && device == dp->device && func == dp->func) {
#if VPCI_DEBUG_IO
			dprintk(KERN_INFO "vpci_read: bus: %02x, devfn: %02x "
				"(device: %02x, func: %02x), reg: %02x, len: %d\n", BUSNUM, devfn, device, func, reg, len);
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
			dprintk(KERN_INFO "vpci_read: value: 0x%08x\n", *value);
#endif
		}
	}

	spin_unlock(&vpci_config_lock);

	return 0;
}

static int vpci_write(struct pci_bus *bus, unsigned int devfn, int reg, int len, u32 value)
{
	struct device_list *dp;
	int rc, device, func;

	device = devfn >> 3;
	func = devfn & 7;

	if (reg + len > 255) return -EINVAL;

	spin_lock(&vpci_config_lock);

	rc = -EPERM;
	for(dp = devices; dp; dp = dp->next) {
		if (BUSNUM == dp->bus && device == dp->device && func == dp->func) {
#if VPCI_DEBUG_IO
			dprintk(KERN_INFO "vpci_write: bus: %02x, devfn: %02x "
				"(device: %02x, func: %02x), reg: %02x, len: %d, value: %08X\n",
				BUSNUM, devfn, device, func, reg, len, value);
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
			dprintk(KERN_INFO "vpci_write: value: 0x%08x\n", value);
#endif
		}
	}

	spin_unlock(&vpci_config_lock);

	return rc;
}

static struct pci_ops vpci_ops = {
	.read = vpci_read,
	.write = vpci_write,
};

static int vpci_next_dev = 0;

int vpci_add_device(int f, int id, int class, int irq) {
	struct device_list *newdev;

	dprintk("VPCI: add_new: f: %d, id: %x, class: %x, irq: %d\n", f, id, class, irq);

	newdev = kzalloc(sizeof(struct device_list), GFP_KERNEL);
	if (!newdev) {
		printk(KERN_ERR "VPCI: no memory for device info!\n");
		return -ENOMEM;
	}
	memset(newdev, 0, sizeof(*newdev));
	newdev->bus = 0;
	newdev->device = vpci_next_dev++;
	newdev->func = f;
	pci_short(newdev->regs, PCI_VENDOR_ID) = PCI_VENDOR_ID_VIRT;
	pci_short(newdev->regs, PCI_DEVICE_ID) = id;
	pci_short(newdev->regs, PCI_COMMAND) = PCI_COMMAND_FAST_BACK;
	pci_short(newdev->regs, PCI_STATUS) = (PCI_STATUS_FAST_BACK | PCI_STATUS_DEVSEL_FAST);
	pci_short(newdev->regs, PCI_HEADER_TYPE) = 0x80;
	pci_short(newdev->regs, PCI_CLASS_DEVICE) = class;
	pci_byte(newdev->regs, PCI_INTERRUPT_LINE) = irq;
	pci_byte(newdev->regs, PCI_INTERRUPT_PIN) = 1;

	spin_lock(&vpci_config_lock);
	if (devices) {
		last_device->next = newdev;
		last_device = newdev;
	} else {
		devices = newdev;
		last_device = newdev;
	}
	spin_unlock(&vpci_config_lock);

	/* Tell bus to scan for devices */
	dprintk("VPCI: re-scanning...\n");
	pci_rescan_bus(vpci_bus);
	return 0;
}
EXPORT_SYMBOL(vpci_add_device);

extern struct pci_raw_ops *raw_pci_ops;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
static int __init vpci_init(void) {

	dprintk(KERN_INFO "VPCI: Initializing\n");

	spin_lock_init(&vpci_config_lock);
	vpci_bus = pci_scan_bus_parented(NULL, 0, &vpci_ops, NULL);
	if (!vpci_bus) {
		printk(KERN_ERR "VPCI: unable to create root bus!\n");
		return -ENOSYS;
	}

	return 0;
}
arch_initcall(vpci_init);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
#include <linux/platform_device.h>
static int vpci_probe(struct platform_device *pdev) {
	LIST_HEAD(resources);

	dprintk(KERN_INFO "VPCI: Initializing\n");

	spin_lock_init(&vpci_config_lock);
#if 0
struct pci_bus *pci_scan_root_bus(struct device *parent, int bus,
                                             struct pci_ops *ops, void *sysdata,
                                             struct list_head *resources);
#endif
	vpci_bus = pci_scan_root_bus(NULL, 0, &vpci_ops, 0, &resources);
	if (!vpci_bus) {
		printk(KERN_ERR "VPCI: unable to create root bus!\n");
		return -ENOSYS;
	}
	pci_bus_add_devices(vpci_bus);
	return 0;
}

static int __init vpci_init(void) {
	return vpci_probe(0);
}
subsys_initcall(vpci_init);

static struct platform_driver gen_pci_driver = {
        .driver = {
                .name = "vpci",
//                .of_match_table = gen_pci_of_match,
        },
        .probe = vpci_probe,
};
module_platform_driver(gen_pci_driver);
#endif

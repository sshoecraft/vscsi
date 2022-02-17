
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/pci_regs.h>
#include <linux/init.h>
#include <linux/irq.h>
#include "vpci.h"
#include "pci.h"

#define VPCI_VERSION "1.00"

MODULE_AUTHOR("Steve Shoecraft <sshoecraft@earthlink.net>");
MODULE_DESCRIPTION("Virtual PCI Bus Driver " VPCI_VERSION);
MODULE_LICENSE("GPL");

#define VPCI_DEBUG 1
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

#define pci_byte(r,l) *((unsigned char *)(&r[l]))
#define pci_short(r,l) *((unsigned short *)(&r[l]))
#define pci_long(r,l) *((unsigned long *)(&r[l]))

#if VPCI_DEBUG
#define dprintk(format, args...) printk("%s(%d): " format ,__FUNCTION__,__LINE__, ## args)
//, printk(format, ## args)
#else
#define dprintk(format, args...) /* noop */
#endif

#if 0
static void bindump(char *msg, void *bdata, int bytes) {
        int offset,end,x,y;
        static char bline[256];
        register unsigned char *data = bdata, *p;

        printk("%s:\n",msg);
        p = bline;
        offset = 0;
        for(x=0; x < bytes; x += 16) {
                p += sprintf(p, "%04X: ",offset);
                end=(x+16 > bytes ? bytes : x+16);
                for(y=x; y < end; y++)  p += sprintf(p,"%02X ",data[y]);
                for(y=end; y < x+16; y++) p += sprintf(p,"   ");
                p += sprintf(p,"  ");
                for(y=x; y < end; y++) {
                        if (data[y] > 32 && data[y] < 127)
                                p += sprintf(p,"%c",data[y]);
                        else
                                p += sprintf(p,".");
                }
                p += sprintf(p,"\n");
                printk(bline);
                p = bline;
                offset += 16;
        }
}
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

#if 0
struct pci_bus * __devinit pcibios_scan_root(int busnum)
{
        struct pci_bus *bus = NULL;

//        dmi_check_system(pciprobe_dmi_table);

        while ((bus = pci_find_next_bus(bus)) != NULL) {
                if (bus->number == busnum) {
                        /* Already scanned */
                        return bus;
                }
        }

        dprintk(KERN_DEBUG "PCI: Probing PCI hardware (bus %02x)\n", busnum);

        return pci_scan_bus_parented(NULL, busnum, &vpci_ops, NULL);
}

static void __devinit pcibios_fixup_peer_bridges(void)
{
        int n, devfn;
	int pcibios_last_bus = -1;

        if (pcibios_last_bus <= 0 || pcibios_last_bus >= 0xff)
                return;
        dprintk("PCI: Peer bridge fixup\n");

        for (n=0; n <= pcibios_last_bus; n++) {
                u32 l;
                if (pci_find_bus(0, n))
                        continue;
                for (devfn = 0; devfn < 256; devfn += 8) {
                        if (!vpci_read(vpci_bus, devfn, PCI_VENDOR_ID, 2, &l) &&
                            l != 0x0000 && l != 0xffff) {
                                printk("Found device at %02x:%02x [%04x]\n", n, devfn, l);
                                printk(KERN_INFO "PCI: Discovered peer bus %02x\n", n);
                                pci_scan_bus(n, &vpci_ops, NULL);
                                break;
                        }
                }
        }
}

static int pci_legacy_init(void)
{
//        if (pcibios_scanned++)
//               return 0;

        printk("PCI: Probing PCI hardware\n");
        vpci_bus = pcibios_scan_root(0);
        if (vpci_bus)
                pci_bus_add_devices(vpci_bus);

//        pcibios_fixup_peer_bridges();

        return 0;
}

static int first_device = 0;
#endif

//int vpci_add_device(int b, int d, int f, int id, int class, int irq) {
int vpci_add_device(int f, int id, int class, int irq) {
	struct device_list *newdev;

	dprintk("VPCI: add_new: f: %d, id: %x, class: %x, irq: %d\n", f, id, class, irq);

	newdev = kzalloc(sizeof(struct device_list), GFP_KERNEL);
	if (!newdev) {
		printk(KERN_ERR "VPCI: no memory for device info!\n");
		return -ENOMEM;
	}
	memset(newdev, 0, sizeof(*newdev));
#if 0
	newdev->bus = b;
	newdev->device = d;
#endif
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

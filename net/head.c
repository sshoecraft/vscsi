
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/pci.h>
#include <linux/cdrom.h>
#include "../pci/vpci.h"

#include <asm/irq.h>

extern int os_net_get_mac(int,unsigned char *);
extern int os_net_tx(int, void *, int);

#define DRV_NAME 	"vnet"
#define DRV_VERSION	"1.00"

MODULE_AUTHOR("Steve Shoecraft <sshoecraft@earthlink.net>");
MODULE_DESCRIPTION("Virtual Network Adapter " DRV_VERSION);
MODULE_LICENSE("GPL");
//MODULE_ALIAS("platform:vscsi");

struct vnet_priv {
	struct net_device *dev;
	struct net_device_stats stats;
	int unit;
	int host_unit;
	unsigned short flags;
	struct pci_dev *pdev;
	spinlock_t ioctl_lock;
	struct mii_if_info mii_if;
};

#define VNET_FLAG_ENABLED	0x01
#define VNET_FLAG_HANDLING	0x02
#define VNET_FLAG_DEBUG	0x80

#define VNET_NUM_UNITS 8
//static struct net_device *vnet_dev[VNET_NUM_UNITS];
static struct vnet_priv *vnet_dev[VNET_NUM_UNITS];

#define VNET_DEBUG 1
#if VNET_DEBUG
#define dprintk(fmt, args...) printk(fmt, ## args)
#else
#define dprintk(fmt, args...) /* noop */
#endif

static void vnet_rx(struct net_device *, void *, int);
static spinlock_t vnet_irq_lock;

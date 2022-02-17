
#include <linux/pci.h>
#include "vpci.h"

static int get_mac(int unit, unsigned char *address)
{
	unsigned long flags;
	co_network_request_t *net_request;
	int result;

	co_passage_page_assert_valid();
	co_passage_page_acquire(&flags);
	co_passage_page->operation = CO_OPERATION_DEVICE;
	co_passage_page->params[0] = CO_DEVICE_NETWORK;
	net_request = (typeof(net_request))&co_passage_page->params[1];
	net_request->unit = unit;
	net_request->type = CO_NETWORK_GET_MAC;
	co_switch_wrapper();
	memcpy(address, net_request->mac_address, 6);
	result = net_request->result;
	co_passage_page_release(flags);

	return result;
}

#if 0
static int get_irq(int type) {
	unsigned long flags;
	co_network_request_t *net_request;
	int result, irq;

	co_passage_page_assert_valid();
	co_passage_page_acquire(&flags);
	co_passage_page->operation = CO_OPERATION_IRQ;
	co_passage_page->params[0] = type;
	co_switch_wrapper();
	irq = co_passage_page->params[0];
	co_passage_page_release(flags);
	result = co_passage_page->params[1];


	return result;
}
#endif

int vpci_co_init(void) {
	/* Get our config */
	co_passage_page_assert_valid();
	co_passage_page_acquire(&flags);
	co_passage_page->operation = CO_OPERATION_DEVICE;
	co_passage_page->params[0] = CO_DEVICE_PCI;
	co_passage_page->params[1] = VPCI_GET_CONFIG;
	co_passage_page->params[2] = 0;
	co_switch_wrapper();
	count = co_passage_page->params[0];
	host_cp = (vpci_config_t *) &co_passage_page->params[1];
	x = count * sizeof(*cp);
	guest_cp = kmalloc(x, GFP_KERNEL);
	memcpy(guest_cp, host_cp, x);
	co_passage_page_release(flags);

	cp = guest_cp;
	for(x=0; x < count; x++, cp++) {
		switch(cp->type) {
		case CO_DEVICE_VIDEO:
			id = PCI_DEVICE_ID_COVIDEO;
			class = PCI_CLASS_DISPLAY_OTHER;
			irq = 0;
			break;
		case CO_DEVICE_AUDIO:
			id = PCI_DEVICE_ID_COAUDIO;
			class = PCI_CLASS_MULTIMEDIA_AUDIO;
			irq = SOUND_IRQ;
			break;
		case CO_DEVICE_SCSI:
			id = PCI_DEVICE_ID_COSCSI;
			class = PCI_CLASS_STORAGE_SCSI;
			irq = SCSI_IRQ;
			break;
#ifdef CO_DEVICE_IDE
		case CO_DEVICE_IDE:
			id = PCI_DEVICE_ID_COIDE;
			class = PCI_CLASS_STORAGE_IDE;
			irq = 0x14;
			break;
#endif
		case CO_DEVICE_NETWORK:
			id = PCI_DEVICE_ID_CONET;
			class = PCI_CLASS_NETWORK_ETHERNET;
			irq = NETWORK_IRQ;
			break;
		default:
			id = class = irq = 0;
		}
		if (id) {
			vpci_add_device(0, cp->dev, cp->func, id, class, cp->type, irq);
			pci_byte(last_device->regs, PCI_CO_UNIT) = cp->unit;
		}
	}
	kfree(guest_cp);

	/* For each network device, get the HW address */
	for(dp = devices; dp; dp = dp->next) {
		if (dp->type == CO_DEVICE_NETWORK) {
			unit = pci_byte(dp->regs, PCI_CO_UNIT);
			if (get_mac(unit, addr) != 0) {
#if VPCI_DEBUG
				printk(KERN_INFO "VPCI: got MAC for host unit %d\n", unit);
#endif
				pci_byte(dp->regs, PCI_CO_MAC1) = addr[0];
				pci_byte(dp->regs, PCI_CO_MAC2) = addr[1];
				pci_byte(dp->regs, PCI_CO_MAC3) = addr[2];
				pci_byte(dp->regs, PCI_CO_MAC4) = addr[3];
				pci_byte(dp->regs, PCI_CO_MAC5) = addr[4];
				pci_byte(dp->regs, PCI_CO_MAC6) = addr[5];
			}
		}
	}
}

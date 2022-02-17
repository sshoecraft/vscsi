#ifndef __VIRT_VPCI_H
#define __VIRT_VPCI_H

#define PCI_VENDOR_ID_VIRT	0x1a55

#define PCI_DEVICE_ID_VSCSI	0x1000
#define PCI_DEVICE_ID_VIDE	0x1001
#define PCI_DEVICE_ID_VNET	0x1002
#define PCI_DEVICE_ID_VVID	0x1003
#define PCI_DEVICE_ID_VSND	0x1004

int vpci_add_device(int func, int id, int class, int irq);

#endif

#
# Makefile for the Linux Virtual device drivers.
#

obj-$(CONFIG_VPCI)		+= pci/
obj-$(CONFIG_VNET)		+= net/
obj-$(CONFIG_VSCSI)		+= scsi/

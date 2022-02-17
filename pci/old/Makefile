#
# Makefile for the Linux vpci SCSI driver.
#
# $Id: //depot/linux-aic79xx-2.5.0/drivers/scsi/vpci/Makefile#8 $
#

CONFIG_VPCI=m

vpci-y			+= init.o common.o pcibios.o
vpci-$(CONFIG_UML)	+= dummy_io.o
obj-$(CONFIG_VPCI)	+= vpci.o

KERN=/usr/src/uml
all:
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd`

modules clean:
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd` $@

clean-files := $(obj)/Module.symvers

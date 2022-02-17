#
# Makefile for the Linux vpci SCSI driver.
#
# $Id: //depot/linux-aic79xx-2.5.0/drivers/scsi/vpci/Makefile#8 $
#

#obj-$(CONFIG_UML) += vpci.o dummy_io.o pcibios.o common.o legacy.o
obj-$(CONFIG_UML) += vpci.o pcibios.o dummy_io.o
obj-$(CONFIG_COOPERATIVE) += vpci.o dummy_io.o pcibios.o init_co.o

KERN=/usr/src/uml-4.3.5
all:
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd`

modules clean:
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd` $@

clean-files := $(obj)/Module.symvers

#
# Makefile for the Linux vnet SCSI driver.
#
# $Id: //depot/linux-aic79xx-2.5.0/drivers/scsi/vnet/Makefile#8 $
#

SRCS = $(src)/head.c

# Host mode
ifeq ($(CONFIG_COOPERATIVE),y)
SRCS += $(src)/co.c
else
ifeq ($(CONFIG_UML),y)
SRCS += $(src)/uml.c
else
SRCS += $(src)/net.c
endif
endif

SRCS += $(src)/core.c $(src)/setup.c $(src)/pci.c

KERN=/usr/src/uml
all:
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd`

modules clean:
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd` $@

obj-$(CONFIG_VNET) += vnet.o

$(obj)/vnet.c: $(SRCS) Makefile
	cat $(SRCS) > $(obj)/vnet.c

clean-files := $(obj)/vnet.c Module.symvers

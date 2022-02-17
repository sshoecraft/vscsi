#
# Makefile for the Linux vscsi driver.
#

FILE := $(src)/file_pre.c $(src)/file_net.c $(src)/file_local.c $(src)/file_uml.c $(src)/file_post.c


ADAPTER=$(src)/adapter.c
SRCS := $(src)/head.c $(src)/dump.c $(src)/response.c $(FILE) $(src)/rom.c $(src)/device.c $(src)/check.c
SRCS += $(src)/ops.c $(src)/queue.c $(src)/config.c $(src)/reset.c $(src)/spi.c $(ADAPTER) $(src)/setup.c
ifeq ($(CONFIG_PCI),y)
SRCS += $(src)/driver_pci.c
else
SRCS += $(src)/driver.c
endif

# Host mode
#ifeq ($(CONFIG_COOPERATIVE),y)
#SRCS += $(src)/host_co.c
#else
#ifeq ($(CONFIG_UML),y)
#SRCS += $(src)/host_uml.c
#else
#SRCS += $(src)/net.c
#endif
#endif

#SRCS += $(src)/core.c $(src)/setup.c $(src)/pci.c

#	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
#	cd $(KERN) && make V=1 drivers/virt/scsi/vscsi.o
#KERN=/usr/src/uml-2.6.32
KERN=/usr/src/uml-4.3.5
all:
	make V=1 -I`pwd` -C $(KERN) SUBDIRS=`pwd`

modules clean:
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd` $@

obj-y += vscsi.o

#	$(obj)/mkdrv $(src)/mkdrv.conf
$(obj)/vscsi.c: $(SRCS) $(src)/Makefile
	cat $(SRCS) > $(obj)/vscsi.c

$(obj)/mkdrv: $(src)/mkdrv.c
	$(CC) -o $(obj)/mkdrv $(src)/mkdrv.c

clean-files := $(obj)/vscsi.c $(obj)/ops.c Module.symvers

$(src)/rom.c:: $(src)/rom/mkrom.c
	(cd $(src)/rom; make)

$(src)/ops.c::
	(cd $(src)/ops; make)

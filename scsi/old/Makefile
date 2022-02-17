
SRCS := head.c bindump.c dump.c response.c file.c rom.c device.c check.c
SRCS += mode_sense.c read_write.c pass.c
include Makefile.srcs
SRCS += table.c queue.c config.c reset.c adapter.c setup.c driver.c
KERN=/lib/modules/`uname -r`/build
#KERN=/usr/src/old/linux-2.6.18.8

OBJS=vscsi.o

#	make -I`pwd` -C /lib/modules/`uname -r`/build SUBDIRS=`pwd` modules

#	make -I`pwd` -C $(KERN) SUBDIRS=`pwd` KBUILD_VERBOSE=1 modules
all: vscsi.c
	make -I`pwd` -C $(KERN) SUBDIRS=`pwd` modules

vscsi.c: $(SRCS)
	cat $(SRCS) > vscsi.c

table.c: op-num.txt mktable
	./mktable

mktable: mktable.c
	cc -o mktable mktable.c

op-num.txt:
	wget http://www.t10.org/lists/op-num.txt

#modules:
#	gcc -D__KERNEL__ -I/usr/src/linux-2.4/include -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common  -Wno-unused -fomit-frame-pointer -pipe -freorder-blocks -mpreferred-stack-boundary=2 -march=pentium3 -DMODULE -DMODVERSIONS -include /usr/src/linux-2.4/include/linux/modversions.h -I/usr/src/linux-2.4/drivers/scsi -nostdinc -iwithprefix include -DKBUILD_BASENAME=ibmphp_res  -c -o vscsi.o vscsi.c

test: vscsi.c
	(cd ../linux-2.6.25.9; sh m)
#	insmod vscsi.ko vscsi=disk:/test/fedora/fedora.img,cd:/test/coe.iso
#	rmmod vscsi

clean:
	rm -f $(CLEANFILES) mktable vscsi.c *.ko $(OBJS) *.mod.* Module.symvers

obj-m := $(OBJS)

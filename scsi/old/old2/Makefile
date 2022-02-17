
SRCS=scsi.pre inquiry.c ready.c table.c
CLEANFILES=table.c mktable scsi.c

obj-y := scsi.o
obj-m := scsi.o

all:
        make -C /lib/modules/`uname -r`/build SUBDIRS=`pwd` modules

scsi.c: $(SRCS)
	cat $(SRCS) > scsi.c

table.c: op-num.txt mktable
	./mktable

mktable: mktable.c
	cc -o mktable mktable.c

asc-num.txt:
	wget http://www.t10.org/lists/asc-num.txt

op-num.txt:
	wget http://www.t10.org/lists/op-num.txt

$(OBJS): internal.h scsi.h

clean:
	rm -f $(CLEANFILES) *.o *.ko $(OBJS) *.mod.* Module.symvers


SRCS=all.c disk.c dvd_rw.c changer.c tape.c

all: rom.o
	cp rom.c ..

rom.o: rom.c rom.h
	cc -c rom.c

rom.c: mkrom
	./mkrom

mkrom: mkrom.c mkrom.h $(SRCS)
	cc -o mkrom mkrom.c

clean:
	rm -f mkrom rom.o rom.c

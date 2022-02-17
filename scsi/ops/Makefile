
TYPES=D
CAPS=MO

all: ../ops.c

../ops.c: op-num.txt mkops func.map file.map *.c Makefile
	./mkops $(TYPES) $(CAPS) && cp ops.c .. && cp ops.h ..

mkops: mkops.c list.c op-num.txt
	cc -Wall -I. -o mkops mkops.c list.c

op-num.txt:
	wget http://www.t10.org/lists/op-num.txt

clean:
	rm -f ../ops.c mkops

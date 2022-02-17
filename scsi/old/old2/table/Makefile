
all: ../table.c

../table.c: op-num.txt mktable
	./mktable && mv table.c ..

mktable: mktable.c
	cc -o mktable mktable.c

asc-num.txt:
	wget http://www.t10.org/lists/asc-num.txt

op-num.txt:
	wget http://www.t10.org/lists/op-num.txt

clean:
	rm -f ../table.c

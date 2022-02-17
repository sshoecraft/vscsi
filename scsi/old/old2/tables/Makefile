
#.SILENT:

SRCS=vendors.c opcodes.c scodes.c
INCS=vendors.h opcodes.h scodes.h

all: ../tables.c ../tables.h

../tables.c: prefix.c $(SRCS) postfix.c
	@sh -c "set clobber; cat prefix.c $(SRCS) postfix.c > ../tables.c"

../tables.h: prefix.h $(INCS) postfix.h
	@sh -c "set clobber; cat prefix.h $(INCS) postfix.h > ../tables.h"

vendorid.txt:
	wget ftp://ftp.t10.org/t10/vendorid.txt

vendors.c vendors.h: mkvendors.c vendorid.txt
	@cc -o tmp mkvendors.c
	@./tmp
	@rm -f tmp

op-num.txt:
	wget http://www.t10.org/lists/op-num.txt

asc-num.txt:
	wget http://www.t10.org/lists/asc-num.txt

opcodes.c: mkopcodes.c op-num.txt asc-num.txt
	@cc -o tmp mkopcodes.c
	@./tmp
	@rm -f tmp

scodes.c scodes.h: mkscodes.c
	@cc -o tmp mkscodes.c
	@./tmp
	@rm -f tmp

clean:
	rm -f $(SRCS) vendorid.txt op-num.txt asc-num.txt ../tables.c ../tables.h

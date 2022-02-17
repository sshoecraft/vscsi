
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	FILE *fp;
	char line[128];
	int val,byte,bit,mask;
	unsigned char bm[16];
	register int x;

	fp = fopen(argv[1],"r");
	if (!fp) return 1;
	memset(bm,0,sizeof(bm));
	while(fgets(line,sizeof(line),fp)) {
		line[strlen(line)-1] = 0;
		val = atoi(line);
		byte = val / 8;
		bit = val % 8;
		mask = 1 << bit;
//		printf("val: %d, byte: %d, bit: %d, mask: %x\n", val, byte, bit, mask);
		bm[byte] |= mask;
	}
	fclose(fp);
	printf("unsigned char %s_commands[16] = { ", argv[1]);
	for(x=0; x < 16; x++) printf("0x%02X ", bm[x]);
	printf("};\n");
	return 0;
}

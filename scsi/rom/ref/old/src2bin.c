
#include <stdio.h>
#include <string.h>
#include "rom.c"

int writepage(char *file, unsigned char *data, int len) {
	FILE *fp;

	fp = fopen(file,"wb+");
	if (!fp) {
		char msg[256];

		sprintf(msg,"fopen %s", file);
		perror(msg);
		return 1;
	}
	fwrite(data,len,1,fp);
	fclose(fp);
}

void src2bin(struct vscsi_rom *rom) {
	FILE *fp;
	struct vscsi_rom_page *page;
	char name[32];

	/* Write out rev */
	sprintf(name,"%s_rev", rom->name);
	fp = fopen(name,"w+");
	if (fp) {
		fprintf(fp, "%s\n", rom->rev);
		fclose(fp);
	}

	/* Write out std page */
	sprintf(name,"%s_std", rom->name);
	writepage(name,rom->std.data,rom->std.size);

	/* Write out vpd pages */
	for(page = rom->vpd; page->data; page++) {
		sprintf(name,"%s_vpd_%02x", rom->name, page->num);
		writepage(name,page->data,page->size);
	}

	/* Write out vpd pages */
	for(page = rom->mode; page->data; page++) {
		sprintf(name,"%s_mode_%02x", rom->name, page->num);
		writepage(name,page->data,page->size);
	}
}

int main(void) {
	src2bin(&disk_rom);
	src2bin(&cd_rom);
}

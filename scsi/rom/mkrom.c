
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mkrom.h"

#define DISK_SUPPORT 1
#define SAN_SUPPORT 0
#define CD_SUPPORT 0
#define DVD_SUPPORT 0
#define CHANGER_SUPPORT 0
#define TAPE_SUPPORT 0

#include "all.c"
#if DISK_SUPPORT
#include "disk.c"
//#include "disk2.c"
#endif
#if SAN_SUPPORT
#include "open_v.c"
#endif
#if CD_SUPPORT
#include "cd.c"
#endif
#if DVD_SUPPORT
#include "dvd_rw.c"
#endif
#if CHANGER_SUPPORT
#include "changer.c"
#endif
#if TAPE_SUPPORT
#include "tape.c"
#endif

#define OUT "rom.c"

/* implemented version */
#define VERSION 0x06

/* response format */
#define FORMAT 0x02

void copy_string(unsigned char *p, char *s, int max) {
	int i;

	strcpy(p,s);
	for(i = strlen(s); i < max; i++) p[i] = ' ';
}

static void hexdump(FILE *fp, unsigned char *p, int len) {
        int x,y;

	fprintf(fp,"\t");
        for(x=0; x < len; x++) {
                if ((x % 8) == 0 && x)
                        fprintf(fp,"\t/* %02x - %02x */\n\t", x-8,x-1);
                fprintf(fp,"0x%02x,", *p);
                p++;
        }
        if ((x % 8) != 0) {
                y = x;
                while((y % 8) != 0) {
                        fprintf(fp,"     ");
                        y++;
                }
                fprintf(fp,"\t/* %02x - %02x */\n", y-8,x-1);
        } else
                fprintf(fp,"\t/* %02x - %02x */\n", x-8,x-1);
}

int write_std(FILE *fp, struct standard_page *std) {
	unsigned char page[96],low,high;

	/* Build the standard page */
	memset(page,0,sizeof(page));
	page[0] = std->PQ << 4 | std->TYPE;
	page[1] = (std->RMB ? 0x80 : 0);
	page[2] = VERSION;
	/* NORMACA HISUP RESPONSE DATA FORMAT */
	page[3] = std->NORMACA << 5 | std->HISUP << 4 | FORMAT;
	page[4] = 91;
	/* SCCS | ACC | TPGS (2) | 3PC | Reserved (2) | PROTECT */
	page[5] = std->SCCS << 7 | std->ACC << 6 | std->TPGS << 4 | std->_3PC << 3 | std->PROTECT;
	/* Obsolete ENCSERV VS MULTIP Obsolete Obsolete Obsolete ADDR16a */
	page[6] = std->ENCSERV << 6 | std->VS << 5 | std->MULTIP << 4 | std->ADDR16;
	/* Obsolete Obsolete WBUS16a SYNCa Obsolete Obsolete CMDQUE VS */
	page[7] = std->WBUS16 << 5 | std->SYNC << 4 | std->CMDQUE << 1 | std->VS2;
	copy_string(&page[8],std->VENDOR,8);
	copy_string(&page[16],std->PRODUCT,16);
	copy_string(&page[32],std->REVISION,4);
	/* Reserved CLOCKINGa QASa IUSa */
	page[56] = std->CLOCKING << 2 | std->QAS << 1 | std->IUS;
	page[58] = std->VERSION1 >> 8;
	page[59] = std->VERSION1 & 0xFF;
	page[60] = std->VERSION2 >> 8;
	page[61] = std->VERSION2 & 0xFF;
	page[62] = std->VERSION3 >> 8;
	page[63] = std->VERSION3 & 0xFF;
	page[64] = std->VERSION4 >> 8;
	page[65] = std->VERSION4 & 0xFF;
	page[66] = std->VERSION5 >> 8;
	page[67] = std->VERSION5 & 0xFF;
	page[68] = std->VERSION6 >> 8;
	page[69] = std->VERSION6 & 0xFF;
	page[70] = std->VERSION7 >> 8;
	page[71] = std->VERSION7 & 0xFF;
	page[72] = std->VERSION8 >> 8;
	page[73] = std->VERSION8 & 0xFF;

	fprintf(fp,"static unsigned char %s_std_page[] = {\n", std->PRODUCT);
	hexdump(fp,page,96);
	fprintf(fp,"};\n\n");

#if 0
	{
		char xname[256];
		FILE *xp;

		sprintf(xname,"%s_std",std->PRODUCT);
		xp = fopen(xname,"wb+");
		if (!xp) {
			perror("fopen _std");
			exit(1);
		}
		fwrite(page,1,sizeof(page),xp);
		fclose(xp);
	}
#endif
	return 0;
}

int cmpnum(const void *a, const void *b) {
	const struct rom_page *p1 = a, *p2 = b;

	if (p1->num < p2->num)
		return -1;
	else if (p1->num == p2->num)
		return 0;
	else
		return 1;
}

void write_pages(FILE *fp, char *name, struct rom_page *pages, int count, int vpd) {
	int x;

	/* Write each page */
	for(x=0; x < count; x++) {
		fprintf(fp,"/* %s %s Page */\n", pages[x].name, (vpd ? "VPD" : "Mode"));
		fprintf(fp,"static unsigned char %s_%s_%02x[] = {\n", name, (vpd ? "vpd" : "mode"), pages[x].num);
		if (vpd) pages[x].data[0] = vpd-1;
		hexdump(fp,pages[x].data,pages[x].len);
		fprintf(fp,"};\n\n");
	}

	/* Write page list */
	fprintf(fp,"/* %s Supported %s Page List */\n", name, (vpd ? "VPD" : "Mode"));
	fprintf(fp,"static struct vscsi_rom_page %s_%s_pages[] = {\n", name, (vpd ? "vpd" : "mode"));
	if (vpd) fprintf(fp,"\tVSCSI_ROM_PAGE(0x00, %s_vpd_00),\t\t/* Supported Pages */\n", name);
	for(x=0; x < count; x++) 
		fprintf(fp,"\tVSCSI_ROM_PAGE(0x%02x, %s_%s_%02x),\t\t/* %s */\n",
			pages[x].num, name, (vpd ? "vpd" : "mode"), pages[x].num, pages[x].name);
	fprintf(fp,"\tVSCSI_ROM_PAGE(0,0)\n");
	fprintf(fp,"};\n\n");
}

void write_vpd(FILE *fp, struct rom_page *pages, char *name, int pq_dt) {
	int x,count,i;
	struct rom_page *vpd,*p;
	unsigned char *data;

	/* Get a count of all pages + device pages */
	count = 0;
	for(x=0; all_vpd_pages[x].data; x++) count++;
	for(x=0; pages[x].data; x++) count++;
//	printf("vpd count: %d\n", count);

	/* Wrap all pages into a single struct */
	vpd = malloc(sizeof(struct rom_page) * count);
	if (!vpd) {
		perror("malloc vpd\n");
		return;
	}
	data = malloc(count);
	if (!data) {
		perror("malloc vpd data\n");
		return;
	}
	p = vpd;
	for(x=0; all_vpd_pages[x].data; x++) {
		memcpy(p, &all_vpd_pages[x], sizeof(struct rom_page));
		p++;
	}
	for(x=0; pages[x].data; x++) {
		memcpy(p, &pages[x], sizeof(struct rom_page));
		p++;
	}

	/* Sort by num */
	qsort(vpd, count, sizeof(struct rom_page), cmpnum);

	/* Write supported pages page */
	memset(data,0,count);
	data[0] = pq_dt;
	data[3] = count + 1;
	i = 5;
	for(x=0; x < count; x++) {
		data[i] = vpd[x].num;
		i++;
	}
	fprintf(fp,"/* Supported VPD Pages */\n");
	fprintf(fp,"static unsigned char %s_vpd_00[] = {\n", name);
	hexdump(fp,data,i);
	fprintf(fp,"};\n");
	fprintf(fp,"\n");

	/* Write the pages */
	write_pages(fp,name,vpd,count,pq_dt+1);

	/* Free alloc'd mem */
	free(vpd);
	free(data);
}

void write_mode(FILE *fp, char *name, struct rom_page *pages) {
	int x,count,i;
	struct rom_page *mode,*p;

	/* Get a count of all pages + device pages */
	count = 0;
	for(x=0; all_mode_pages[x].data; x++) count++;
	for(x=0; pages[x].data; x++) count++;
//	printf("mode count: %d\n", count);

	/* Wrap all pages into a single struct */
	mode = malloc(sizeof(struct rom_page) * count);
	if (!mode) {
		perror("malloc mode\n");
		return;
	}
	p = mode;
	for(x=0; all_mode_pages[x].data; x++) {
		memcpy(p, &all_mode_pages[x], sizeof(struct rom_page));
		p++;
	}
	for(x=0; pages[x].data; x++) {
		memcpy(p, &pages[x], sizeof(struct rom_page));
		p++;
	}

	/* Sort by num */
	qsort(mode, count, sizeof(struct rom_page), cmpnum);

	/* Write the pages */
	write_pages(fp,name,mode,count,0);

	/* Free alloc'd mem */
	free(mode);
}

int write_rom(FILE *fp, struct rom_info *info) {
	int pq_dt;
	char name[32];

	strcpy(name,info->std->PRODUCT);
	pq_dt = info->std->PQ << 4 | info->std->TYPE;
	write_std(fp, info->std);
	write_vpd(fp, info->vpd_pages, name, pq_dt);
	write_mode(fp, name, info->mode_pages);

	fprintf(fp,"/* %s ROM Definition */\n", name);
	fprintf(fp,"static struct vscsi_rom %s_rom = {\n", name);
	fprintf(fp,"\t%d,\n", info->sector_size);
	fprintf(fp,"\tVSCSI_ROM_PAGE(0, %s_std_page),\n", name);
	fprintf(fp,"\t%s_vpd_pages,\n", name);
	fprintf(fp,"\t%s_mode_pages\n", name);
	fprintf(fp,"};\n\n");
}

int main(void) {
	struct rom_info *info;
	FILE *fp,*fp2;
	char line[128];

	fp = fopen(OUT,"w+");
	if (!fp) {
		perror("fopen rom.c!\n");
		return 1;
	}
	fp2 = fopen("rom.h","r+");
	if (!fp2) {
		perror("fopen rom.h!\n");
		return 1;
	}
	while(fgets(line,sizeof(line)-1,fp2)) fprintf(fp,"%s",line);
	fclose(fp2);

#if DISK_SUPPORT
	write_rom(fp, &disk_info);
#endif
#if DVD_SUPPORT
	write_rom(fp, &dvd_rw_info);
#endif
#if CHANGER_SUPPORT
	write_rom(fp, &changer_info);
#endif
#if TAPE_SUPPORT
	write_rom(fp, &tape_info);
#endif

	fclose(fp);
	return 0;
}

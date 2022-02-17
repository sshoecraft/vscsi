
struct standard_page {
	int PQ;
	int TYPE;
	unsigned RMB: 1;
	int VERSION;
	unsigned NORMACA: 1;
	unsigned HISUP: 1;
	int RESPONSE_FORMAT;
//	int ADDITIONAL_LENGTH;
	unsigned SCCS: 1;
	unsigned ACC: 1;
	int TPGS;
	unsigned _3PC: 1;
	unsigned PROTECT: 1;
	unsigned ENCSERV: 1;
	unsigned VS: 1;
	unsigned MULTIP: 1;
	unsigned ADDR16: 1;
	unsigned WBUS16: 1;
	unsigned SYNC: 1;
	unsigned CMDQUE: 1;
	unsigned VS2: 1;
	char *VENDOR;
	char *PRODUCT;
	char *REVISION;
	unsigned char SPECIFIC[20];
	unsigned CLOCKING: 2;
	unsigned QAS: 1;
	unsigned IUS: 1;
	unsigned short VERSION1;
	unsigned short VERSION2;
	unsigned short VERSION3;
	unsigned short VERSION4;
	unsigned short VERSION5;
	unsigned short VERSION6;
	unsigned short VERSION7;
	unsigned short VERSION8;
	unsigned char ADDL_SPECIFIC[];
};

struct rom_page {
	char *name;
	int num;
	unsigned char *data;
	int len;
};

#define ROM_PAGE(s,n,p) { s, n, p, sizeof(p) }
#define PAGE_LIST_END { 0, 0, 0, 0 }

//#include "rom.h"

struct rom_info {
	int sector_size;
	struct standard_page *std;
	struct rom_page *vpd_pages;
	struct rom_page *mode_pages;
};

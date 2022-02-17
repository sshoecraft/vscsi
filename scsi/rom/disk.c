
struct standard_page disk_standard_page = {
.PQ			= 0x00,
.TYPE			= 0x00,
.RMB			= 0,
.VERSION		= 0x06,
.NORMACA		= 0,
.HISUP			= 1,
.RESPONSE_FORMAT	= 2,
.SCCS			= 0,
.ACC			= 0,
.TPGS			= 0x00,
._3PC			= 0,
.PROTECT		= 0,
.ENCSERV		= 0,
.VS			= 0,
.MULTIP			= 0,
.ADDR16			= 1,
.WBUS16			= 1,
.SYNC			= 1,
.CMDQUE			= 1,
.VS2			= 0,
.VENDOR			= "vSCSI",
.PRODUCT		= "vDISK",
.REVISION		= "1.00",
.SPECIFIC		=
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
		0x00,0x00,0x00,0x00,				/* 10 - 13 */
	},
.IUS			= 1,
.QAS			= 1,
.CLOCKING		= 0x03,			/* 16 bit data with 16 SCSI IDs */
.VERSION1		= 0x0040,
.VERSION2		= 0x0b54,
.VERSION3		= 0x013c,
.VERSION4		= 0x019b,
.VERSION5		= 0x0000,
.VERSION6		= 0x0000,
.VERSION7		= 0x0000,
.VERSION8		= 0x0000,
};

struct rom_page disk_vpd_pages[] = {
	PAGE_LIST_END
};


unsigned char disk_mode_08[] = {
	/* 0 PS Reserved PAGE CODE (08h) */
	0x08,
	/* 1 PAGE LENGTH (12h) */
	0x12,
	/* 2 IC ABPF CAP DISC SIZE WCE MF RCD */
	0x14,
	/* 3 DEMAND READ RETENTION PRIORITY WRITE RETENTION PRIORITY */
	0x00,
	/* 4 (MSB) DISABLE PRE-FETCH TRANSFER LENGTH */
	0xff,
	/* 5 (LSB) */
	0xff,
	/* 6 (MSB) MINIMUM PRE-FETCH */
	0x00,
	/* 7 (LSB) */
	0x00,
	/* 8 (MSB) MAXIMUM PRE-FETCH */
	0x00,
	/* 9 (LSB) */
	0x00,
	/* 10 (MSB) MAXIMUM PRE-FETCH CEILING */
	0xff,
	/* 11 (LSB) */
	0xff,
	/* 12 FSW LBCSS DRA Vendor-specific Reserved NV_DIS */
	0x80,
	/* 13 NUMBER OF CACHE SEGMENTS */
	0x08,
	/* 14 (MSB) CACHE SEGMENT SIZE */
	0x00,
	/* 15 (LSB) */
	0x00,
	/* 16 Reserved */
	0x00,
	/* 17-19 Obsolete */
	0x00,0x00,0x00
};

struct rom_page disk_mode_pages[] = {
	ROM_PAGE("Caching",0x08,disk_mode_08),
	PAGE_LIST_END
};

struct rom_info disk_info = {
	512,
	&disk_standard_page,
	disk_vpd_pages,
	disk_mode_pages,
};


struct standard_page changer_standard_page = {
.PQ			= 0x00,
.TYPE			= 0x08,
.RMB			= 1,
.VERSION		= 0x02,
.NORMACA		= 0,
.HISUP			= 0,
.RESPONSE_FORMAT	= 2,
.SCCS			= 0,
.ACC			= 0,
.TPGS			= 0x00,
._3PC			= 0,
.PROTECT		= 0,
.ENCSERV		= 0,
.VS			= 0,
.MULTIP			= 0,
.ADDR16			= 0,
.WBUS16			= 0,
.SYNC			= 0,
.CMDQUE			= 0,
.VS2			= 0,
.VENDOR			= "vSCSI",
.PRODUCT		= "vCHANGER",
.REVISION		= "1.00",
.SPECIFIC		=
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
		0x00,0x00,0x00,0x00,
	},
.CLOCKING		= 0x00,
.QAS			= 0,
.IUS			= 0,
.VERSION1		= 0x0000,
.VERSION2		= 0x0000,
.VERSION3		= 0x0000,
.VERSION4		= 0x0000,
.VERSION5		= 0x0000,
.VERSION6		= 0x0000,
.VERSION7		= 0x0000,
.VERSION8		= 0x0000,
};

struct rom_page changer_vpd_pages[] = {
	PAGE_LIST_END
};

struct rom_page changer_mode_pages[] = {
	PAGE_LIST_END
};

struct rom_info changer_info = {
	0,
	&changer_standard_page,
	changer_vpd_pages,
	changer_mode_pages
};

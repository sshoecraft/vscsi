
static struct standard_page disk_std = {
.PQ			= 0x00,
.TYPE			= 0x00,
.RMB			= 0,
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
.WBUS16			= 1,
.SYNC			= 1,
.CMDQUE			= 1,
.VS2			= 1,
.VENDOR			= "vSCSI",
.PRODUCT		= "vDISK",
.REVISION		= "1.0",
.SPECIFIC		=
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 	/* 00 - 07 */
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 	/* 08 - 0f */
		0x00,0x00,0x00,0x00,                    	/* 10 - 13 */
	},
.IUS			= 0,
.QAS			= 0,
.CLOCKING		= 0x00,
.VERSION1		= 0x0000,
.VERSION2		= 0x0000,
.VERSION3		= 0x0000,
.VERSION4		= 0x0000,
.VERSION5		= 0x0000,
.VERSION6		= 0x0000,
.VERSION7		= 0x0000,
.VERSION8		= 0x0000,
};

static struct rom_page disk_vpd_pages[] = {
	PAGE_LIST_END
};

static unsigned char disk_mode_01[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,          	/* 00 - 05 */
};

static unsigned char disk_mode_02[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

static unsigned char disk_mode_03[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 	/* 00 - 07 */
	0x00,0x3f,0x02,0x00,0x00,0x00,0x00,0x00, 	/* 08 - 0f */
	0x00,0x00,0x00,0x00,0x00,0x00,          	/* 10 - 15 */
};

static unsigned char disk_mode_04[] = {
	0x00,0x0c,0x3d,0xff,0x00,0x0c,0x3d,0x00, 	/* 00 - 07 */
	0x00,0x00,0x00,0x00,0x00,0x0c,0x3d,0x00, 	/* 08 - 0f */
	0x00,0x00,0x1c,0x20,0x00,0x00,          	/* 10 - 15 */
};

static unsigned char disk_mode_09[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
};

static unsigned char disk_mode_0a[] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
};

static struct rom_page disk_mode_pages[] = {
	ROM_PAGE("Read-Write Error Recovery",0x01,disk_mode_01),
	ROM_PAGE("Disconnect-Reconnect",0x02,disk_mode_02),
	ROM_PAGE("Format Device",0x03,disk_mode_03),
	ROM_PAGE("Rigid Disk Geometry",0x04,disk_mode_04),
	ROM_PAGE("Peripheral device",0x09,disk_mode_09),
	ROM_PAGE("Control",0x0a,disk_mode_0a),
	PAGE_LIST_END
};

struct rom_info disk_info = {
	512,
	&disk_std,
	disk_vpd_pages,
	disk_mode_pages,
};

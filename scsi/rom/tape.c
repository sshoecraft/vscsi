
struct standard_page tape_standard_page = {
.PQ			= 0x00,
.TYPE			= 0x01,
.RMB			= 1,
.VERSION		= 0x06,
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
.ADDR16			= 1,
.WBUS16			= 1,
.SYNC			= 1,
.CMDQUE			= 0,
.VS2			= 0,
.VENDOR			= "vSCSI",
.PRODUCT		= "vTAPE",
.REVISION		= "1.00",
.SPECIFIC		=
	{
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
		0x00,0x00,0x00,0x00,
	},
.CLOCKING		= 0x03,
.QAS			= 1,
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

#if 0
req'd per ssc3:
00h Supported log pages
02h Write Error Counter log page
03h Read Error Counter log page (read)
04h Read Reverse Error Counter log page
0Ch Sequential-Access Device log page
2Eh TapeAlert log page
struct vscsi_rom_page tape_log_pages[] = {
	VSCSI_ROM_PAGE(0x02,tape_log_02),
	VSCSI_ROM_PAGE(0x03,tape_log_03),
	VSCSI_ROM_PAGE(0x04,tape_log_04),
	VSCSI_ROM_PAGE(0x0C,tape_log_0C),
	VSCSI_ROM_PAGE(0x2E,tape_log_2E),
	PAGE_LIST_END
};
#endif

struct rom_page tape_vpd_pages[] = {
	PAGE_LIST_END
};

#if 0
req'd per ssc3:
01h 00h Read-Write Error Recovery
02h 00h Disconnect-Reconnect
0Ah 00h Control
0Fh 00h Data Compression
10h 00h Device Configuration
1Ch 00h Informational Exceptions Control
1Dh 00h Medium Configuration
#endif

/* Read-Write Error Recovery page */
unsigned char tape_mode_01[] = {
	/* PS << 7 | SPF << 6 | PAGE_CODE (01h) */
	0x00 << 7 | 0x00 << 0 | 0x01,
	/* PAGE LENGTH (0Ah) */
	0x0A,
	/* TB << 5 | EER << 3 | PER << 2 | DTE << 1 | DCR */
	0x00 << 5 | 0x00 << 3 | 0x00 << 2 | 0x00 << 1 | 0x00,
	/* READ RETRY COUNT */
	0x00,
	/* Reserved */
	0x00,
	/* Reserved */
	0x00,
	/* Reserved */
	0x00,
	/* Reserved */
	0x00,
	/* WRITE RETRY COUNT */
	0x00,
	/* Reserved */
	0x00,
	/* Reserved */
	0x00,
	/* Reserved */
	0x00
};

/* Disconnect-Reconnect page */
unsigned char tape_mode_02[] = {
	/* PS << 7 | SPF << 6 | PAGE_CODE (01h) */
	0x00 << 7 | 0x00 << 6 |0x02,
	/* PAGE LENGTH (0Eh) */
	0x0E,
	/* BUFFER FULL RATIO */
	0x00,
	/* BUFFER EMPTY RATIO */
	0x00,
	/* BUS INACTIVITY LIMIT (MSB) */
	0x00,
	/* BUS INACTIVITY LIMIT (LSB) */
	0x00,
	/* DISCONNECT TIME LIMIT (MSB) */
	0x00,
	/* DISCONNECT TIME LIMIT (LSB) */
	0x00,
	/* CONNECT TIME LIMIT (MSB) */
	0x00,
	/* CONNECT TIME LIMIT (LSB) */
	0x00,
	/* MAXIMUM BURST SIZE (MSB) */
	0x00,
	/* MAXIMUM BURST SIZE (LSB) */
	0x00,
	/* EMDP << 7 | FAIR ARBITRATION << 4 | DIMM << 3 | DTDC */
	0x00 << 7 | 0x00 << 4 | 0x00 << 3 | 0x00,
	/* Reserved */
	0x00,
	/* FIRST BURST SIZE (MSB) */
	0x00,
	/* FIRST BURST SIZE (LSB) */
	0x00
};

struct rom_page tape_mode_pages[] = {
	ROM_PAGE("Read-Write Error Recovery",0x01,tape_mode_01),
	ROM_PAGE("Disconnect-Reconnect",0x02,tape_mode_02),
#if 0
	ROM_PAGE(0x0A,tape_mode_0A),
	ROM_PAGE(0x0F,tape_mode_0F),
	ROM_PAGE(0x10,tape_mode_10),
	ROM_PAGE(0x1C,tape_mode_1C),
	ROM_PAGE(0x1D,tape_mode_1D),
#endif
	PAGE_LIST_END
};

struct rom_info tape_info = {
	4096,
	&tape_standard_page,
	tape_vpd_pages,
	tape_mode_pages
};

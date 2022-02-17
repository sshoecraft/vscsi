
/* Unit Serial Number VPD page */
static unsigned char all_vpd_80[] = {
	/* PERIPHERAL QUALIFIER << 4 | PERIPHERAL DEVICE TYPE */
	0x00,
	/* PAGE CODE (80h) */
	0x80,
	/* Reserved */
	0x00,
	/* PAGE LENGTH */
	0x08,
	/* PRODUCT SERIAL NUMBER */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

/* Device Identification VPD page */
static unsigned char all_vpd_83[] = {
	/* PERIPHERAL QUALIFIER << 4 | PERIPHERAL DEVICE TYPE */
	0x00,
	/* PAGE CODE (83h) */
	0x83,
	/* PAGE LENGTH (MSB) */
	0x00,
	/* PAGE LENGTH (LSB) */
	0x0C,

	/* Designator descriptor */
	/* PROTOCOL IDENTIFIER << 4 | CODE SET */
	0x01 << 4 | 0x01,
	/* PIV << 7 | ASSOCIATION << 4 | DESIGNATOR TYPE */
	0x01 << 7 | 0x00 << 4 | 0x03,
	/* Reserved */
	0x00,
	/* Designator length */
	0x08,

	/* Designator */
	/* NAA << 4 | VALUE: 4 */
	0x03 << 4 | 0x00,
	/* LOCALLY ADMINISTERED VALUE */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

struct rom_page all_vpd_pages[] = {
	ROM_PAGE("Unit Serial Number",0x80,all_vpd_80),
	ROM_PAGE("Device Identification",0x83,all_vpd_83),
	ROM_PAGE(0,0,0)
};

/* Control mode page */
unsigned char all_mode_0A[] = {
	/* 0: PS << 7 | SPF << 6 | PAGE_CODE (01h) */
	0x00 << 7 | 0x00 << 6 |0x02,
	/* 1: PAGE LENGTH (0Ah) */
	0x0A,
	/* 2: TST << 5 | TMF_ONLY << 4 | D_SENSE << 2 | GLTSD << 1 | RLEC */
	0x00 << 5 | 0x00 << 4 | 0x00 << 2 | 0x00 << 1 | 0x00,
	/* 3: QUEUE ALGORITHM MODIFIER << 4 | QERR << 1 */
	0x00 << 4 | 0x00 << 1,
	/* 4: VS << 7 | RAC << 6 | UA_INTLCK_CTRL << 4 | SWP << 3 */
	0x00 << 7 | 0x00 << 6 | 0x00 << 4 | 0x00 << 3,
	/* 5: ATO << 7 | TAS << 6 | AUTOLOAD MODE */
	0x00 << 7 | 0x00 << 6 | 0x00,
	/* 6: Obsolete */
	0x00,
	/* 7: Obsolete */
	0x00,
	/* 8: BUSY TIMEOUT PERIOD (MSB) */
	0x00,
	/* 9: BUSY TIMEOUT PERIOD (LSB) */
	0x00,
	/* 10: EXTENDED SELF-TEST COMPLETION TIME (MSB) */
	0x00,
	/* 11: EXTENDED SELF-TEST COMPLETION TIME (LSB) */
	0x00
};

struct rom_page all_mode_pages[] = {
	ROM_PAGE("Control",0x0A,all_mode_0A),
	ROM_PAGE(0,0,0)
};


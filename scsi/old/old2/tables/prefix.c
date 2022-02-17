
#define __HAVE_TABLES
#include "tables.h"

/* Table of device type strings */
static const char *scsi_device_types[SDT_MAX+1] = {
	"Disk",				/* 00h */
	"Tape",				/* 01h */
	"Printer",			/* 02h */
	"Processor",			/* 03h */
	"WORM",				/* 04h */
	"CD-ROM",			/* 05h */
	"Scanner",			/* 06h */
	"Optical",			/* 07h */
	"Changer",			/* 08h */
	"Communications",		/* 09h */
	"Graphics arts device type 1",	/* 0Ah */
	"Graphics arts device type 2",	/* 0Bh */
	"Array Controller",		/* 0Ch */
	"Enclosure",			/* 0Dh */
	"Simplified Disk",		/* 0Eh */
	"Card Reader/Writer",		/* 0Fh */
	"Bridging Expander"		/* 10h */
};

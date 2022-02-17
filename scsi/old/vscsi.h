
#ifndef __VSCSI_H
#define __VSCSI_H

/* Device instance */
struct vscsi_device {
	int id;					/* ID */
	int type;				/* SCSI device type */
	struct vscsi_file *fp;			/* File ptr */
	unsigned short flags;			/* Status flags */
	struct vscsi_rom *rom;			/* ROM */
	int key,asc,asq;			/* Sense info */
};

/* Functions */

#endif

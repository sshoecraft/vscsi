
#if 0
#include <linux/crc32.h>
#ifdef __LITTLE_ENDIAN
#define crcfunc crc32_le
#else
#define crcfunc crc32_be
#endif
#endif

#define VSCSI_NUM_CHANS 2
#define VSCSI_NUM_IDS 4
#define VSCSI_NUM_LUNS 4

#ifdef current
#undef current
#endif

/* Device def */
struct vscsi_device {
	int chan,id,lun;			/* Self reference */
	int type;				/* SCSI device type */
	unsigned short flags;			/* Internal flags */
	struct vscsi_file *fp;			/* File ptr */
	struct vscsi_rom *rom;			/* ROM ptr */
	vscsi_ops_t *ops;			/* Supported ops */
	int key,asc,asq;			/* Sense info */
	void *priv;				/* Device specific info */
};

#define DEVICE_FLAG_INUSE	0x0001	/* Allocated */
#define DEVICE_FLAG_OPEN	0x0002	/* File open */
#define DEVICE_FLAG_RMB		0x0004	/* Removable */
#define DEVICE_FLAG_RO		0x0008	/* Read only */
#define DEVICE_FLAG_FORMAT	0x4000	/* Formatting */
#define DEVICE_FLAG_DEBUG	0x8000	/* Debugging */

#define DFLAG_ISSET(p,f) ( ( (p)->flags & DEVICE_FLAG_##f ) != 0 )
#define DFLAG_SET(p,f) (p)->flags |= DEVICE_FLAG_##f
#define DFLAG_UNSET(p,f) (p)->flags &= ~DEVICE_FLAG_##f

struct vscsi_id {
	struct vscsi_device luns[VSCSI_NUM_LUNS];
};

struct vscsi_channel {
	struct vscsi_id ids[VSCSI_NUM_IDS];
};

struct vscsi_channel channels[VSCSI_NUM_CHANS];
static struct vscsi_device dummy_device;

#define vscsi_device_clear(p) memset((p), 0, sizeof(struct vscsi_device)); (p)->type = 0x7F

#if 0
#define vscsi_get_device(c,i,l) &channels[c].ids[i].luns[l];
#else
static struct vscsi_device *vscsi_get_device(int r_chan, int r_id, int r_lun) {
	struct vscsi_device *dp;

	dprintk("vscsi_get_device: chan: %d(%d), id: %d(%d), lun: %d(%d)\n", 
		r_chan,VSCSI_NUM_CHANS,r_id,VSCSI_NUM_IDS,r_lun,VSCSI_NUM_LUNS);
	dp = 0;
	if (r_chan >= 0 && r_chan < VSCSI_NUM_CHANS) {
		struct vscsi_channel *chan;

		chan = &channels[r_chan];
		if (r_id >= 0 && r_id < VSCSI_NUM_IDS) {
			struct vscsi_id *id;

			id = &chan->ids[r_id];
			if (r_lun >= 0 && r_lun < VSCSI_NUM_LUNS)
				dp = &id->luns[r_lun];
		}
	}

	dprintk("vscsi_get_device: dp: %p\n", dp);
	if (!dp) dp = (struct vscsi_device *) &dummy_device;
	return dp;
}
#endif

static struct vscsi_device *_scan_luns(struct vscsi_id *ids, int r_lun) {
	struct vscsi_device *dp;
	int l;

	dp = 0;
	if (r_lun == -1) {
		for(l=0; l < VSCSI_NUM_LUNS; l++) {
			if (!DFLAG_ISSET(&ids->luns[l],INUSE)) {
				dp = &ids->luns[l];
				dp->lun = l;
				break;
			}
		}
	} else {
		if (!DFLAG_ISSET(&ids->luns[r_lun],INUSE)) {
			dp = &ids->luns[r_lun];
			dp->lun = r_lun;
		}
	}

	if (dp) DFLAG_SET(dp,INUSE);
	return dp;
}

static struct vscsi_device *_scan_ids(struct vscsi_channel *chan, int r_id, int r_lun) {
	struct vscsi_device *dp;
	int i;

	dp = 0;
	if (r_id == -1) {
		for(i=0; i < VSCSI_NUM_IDS; i++) {
			dp = _scan_luns(&chan->ids[i], r_lun);
			if (dp) {
				dp->id = i;
				break;
			}
		}
	} else {
		dp = _scan_luns(&chan->ids[r_id], r_lun);
		dp->id = r_id;
	}

	return dp;
}
		
static struct vscsi_device *vscsi_device_alloc(int r_chan, int r_id, int r_lun) {
	struct vscsi_device *dp;
	int c;

	dp = 0;
	if (r_chan == -1) {
		for(c=0; c < VSCSI_NUM_CHANS; c++) {
			dp = _scan_ids(&channels[c], r_id, r_lun);
			if (dp) {
				dp->chan = c;
				break;
			}
		}
	} else {
		dp = _scan_ids(&channels[r_chan], r_id, r_lun);
		dp->chan = r_chan;
	}

	return dp;
}

static void vscsi_device_free(struct vscsi_device *dp) {
	if (!dp) return;

	/* If file open, close and free it */
	if (dp->fp) {
		vscsi_file_close(dp->fp);
		vscsi_file_free(dp->fp);
	}

	if (!DFLAG_ISSET(dp,INUSE)) return;

	/* Just clear it */
	vscsi_device_clear(dp);
}

static int vscsi_device_open(struct vscsi_device *dp) {
	dprintk("vscsi_device_open: open: %d\n", DFLAG_ISSET(dp,OPEN));
	if (DFLAG_ISSET(dp,OPEN)) return 0;

	/* Open the file */
	if (vscsi_file_open(dp->fp, DFLAG_ISSET(dp,RO)))
		return 1;

	DFLAG_SET(dp,OPEN);
	return 0;
}

static int vscsi_device_close(struct vscsi_device *dp) {

	dprintk("vscsi_device_close: open: %d\n", DFLAG_ISSET(dp,OPEN));
	if (!DFLAG_ISSET(dp,OPEN)) return 1;

	if (vscsi_file_close(dp->fp))
		return 1;

	DFLAG_UNSET(dp,OPEN);
	return 0;
}

static struct vscsi_device *vscsi_add_device(unsigned char type, unsigned char chan, char *path) {
	struct vscsi_device *dp;
//	unsigned long serial;

	dprintk("add_device: chan: %d, type: %d, path: %s\n", chan, type, path);

	dp = vscsi_device_alloc(-1,-1,-1);
	if (!dp) {
		dprintk(KERN_ERR "vSCSI: unable to alloc new device!\n");
		return 0;
	}
//	dprintk("vscsi_add_device: id: %d\n", dp->id);

	/* Alloc an fp for this device */
	dp->fp = vscsi_file_alloc(path,1);
	if (!dp->fp) {
		dprintk(KERN_ERR "vSCSI: unable to alloc new file!\n");
		vscsi_device_free(dp);
		return 0;
	}

	/* XXX */
	DFLAG_SET(dp,DEBUG);

	dp->type = type;
	switch(type) {
#ifdef HAVE_DISK
	case TYPE_DISK:
	case TYPE_MOD:
		dp->rom = &vDISK_rom;
		dp->ops = vscsi_disk_ops;
		break;
#endif
#ifdef HAVE_TAPE
	case TYPE_TAPE:
		dp->rom = &vTAPE_rom;
		dp->ops = vscsi_tape_ops;
		break;
#endif
#ifdef HAVE_CD
	case TYPE_WORM:
	case TYPE_ROM:
		dp->rom = &vDVD_rom;
		dp->ops = vscsi_cd_ops;
		dp->flags |= DEVICE_FLAG_RO;
		break;
#endif
#ifdef HAVE_CHANGER
	case TYPE_MEDIUM_CHANGER:
		dp->rom = &vCHANGER_rom;
		dp->ops = vscsi_changer_ops;
		break;
#endif
	case TYPE_PASS:
		break;
	default:
		dprintk(KERN_ERR "vSCSI: unsupported device type: %d\n", type);
		vscsi_file_free(dp->fp);
		break;
	}

	if (dp->rom) {
		/* Set RMB flag from rom */
		if (dp->rom->std.data[1] & 0x80) dp->flags |= DEVICE_FLAG_RMB;

#if 0
		/* Generate serial from CRC of path */
		serial = crcfunc(~0, (unsigned char *) path, (size_t) strlen(path));
		if (DFLAG_ISSET(dp,DEBUG)) dprintk("vscsi_add_device: %lx\n", serial);

		/* Update the NAA locally-administrated value in the rom */
		if (dp->rom) {
			struct vscsi_rom_page *page;
			int i;
	
			i = 0;
			for(page = dp->rom->vpd; page; page++) {
				if (page->num == 0x83) {
					unsigned char *p = page->data;
					p[12] = (serial >> 24) & 0xff;
					p[13] = (serial >> 16) & 0xff;
					p[14] = (serial >> 8) & 0xff;
					p[15] = serial & 0xff;
					i = 1;
					break;
				}
			}
			if (!i) dprintk(KERN_WARNING "vSCSI: device_add: unable to update NAA value!\n");
		}
#endif
	}

	/* If not removable, try to access the file */
	if (!DFLAG_ISSET(dp, RMB)) {
		if (vscsi_device_open(dp)) {
			dprintk(KERN_ERR "vSCSI: unable to open %s, device not added.\n", path);
			vscsi_file_free(dp->fp);
			vscsi_device_free(dp);
			return 0;
		}
	}

	dprintk("vscsi_add_device: %d:%d:%d\n", dp->chan, dp->id, dp->lun);
	return dp;
}

static int vscsi_device_init(void) {
	struct vscsi_device *dp;
	int c,i,l;

	dprintk("vSCSI: initializing devices...\n");
	for(c=0; c < VSCSI_NUM_CHANS; c++) {
		for(i=0; i < VSCSI_NUM_IDS; i++) {
			for(l=0; l < VSCSI_NUM_LUNS; l++) {
				dp = vscsi_get_device(c,i,l);
				vscsi_device_clear(dp);
			}
		}
	}
	vscsi_device_clear(&dummy_device);
	return 0;
}


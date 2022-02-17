
#define VSCSI_DEBUG_SENSE 1

static int _add_page(int d, unsigned char *p, struct vscsi_rom_page *page) {
	memcpy(p, page->data, page->size);
	dump_data(0, "page data", page->data, page->size);
	return page->size;
}

/* XXX buflen is the amount of bytes REMAINING in the buffer */
static int mode_sense(struct vscsi_device *dp, struct scsi_cmnd *scp, unsigned char *p, int buflen, int dl, int *data_len) {
	struct vscsi_rom_page *pages = dp->rom->mode;
	unsigned long long max_lba;
	unsigned char *oldp;
	int x,page;

	calc_max_lba(dp->fp->size, dp->rom->sector_size, &max_lba);

	/* Save ptr */
	oldp = p;

	/* Generate block descriptor */	
	dprintk("dl: %d\n", dl);
	if (dl == 8) {
		if (dp->type == 0) {
			/* Direct-access device mode parameter block descriptor */
			/* 0-3: NUMBER OF BLOCKS */
			if (max_lba > 0xfffffffe) {
			*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
			} else {
			*p++ = (max_lba >> 24) & 0xff;
				*p++ = (max_lba >> 16) & 0xff;
				*p++ = (max_lba >> 8) & 0xff;
				*p++ = max_lba & 0xff;
			}
		} else {
			/* General mode parameter block descriptor */
			/* 0: DENSITY CODE */
			*p++ = 0;
			/* 1-3: NUMBER OF BLOCKS */
			if (max_lba > 0xfffffe) {
			*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
			} else {
			*p++ = (max_lba >> 16) & 0xff;
				*p++ = (max_lba >> 8) & 0xff;
				*p++ = max_lba & 0xff;
			}
		}
		/* 4: DENSITY CODE/Reserved */
		*p++ = 0;
		/* 5-7: BLOCK LENGTH */
		*p++ = (dp->rom->sector_size >> 16) & 0xff;
		*p++ = (dp->rom->sector_size >> 8) & 0xff;
		*p++ = dp->rom->sector_size & 0xff;
	} else if (dl == 16) {
		/* Long LBA mode parameter block descriptor */
		/* 0-7: NUMBER OF BLOCKS */
		*p++ = (max_lba >> 56) & 0xff;
		*p++ = (max_lba >> 48) & 0xff;
		*p++ = (max_lba >> 40) & 0xff;
		*p++ = (max_lba >> 32) & 0xff;
		*p++ = (max_lba >> 24) & 0xff;
		*p++ = (max_lba >> 16) & 0xff;
		*p++ = (max_lba >> 8) & 0xff;
		*p++ = max_lba & 0xff;
		/* 8: DENSITY CODE */
		*p++ = 0;
		/* 9-11: Reserved */
		*p++ = 0;
		*p++ = 0;
		*p++ = 0;
		/* 12-15: BLOCK LENGTH */
		*p++ = (dp->rom->sector_size >> 24) & 0xff;
		*p++ = (dp->rom->sector_size >> 16) & 0xff;
		*p++ = (dp->rom->sector_size >> 8) & 0xff;
		*p++ = dp->rom->sector_size & 0xff;
	}

	buflen -= (p - oldp);

	page = scp->cmnd[2] & 0x3f;
	dprintk("page: %x\n", page);
	if (page == 0x3f) {
		/* All pages */
		if (scp->cmnd[3] == 0 || scp->cmnd[3] == 0xFF) {
			for(x=0; pages[x].data; x++) {
				if (pages[x].size < buflen) {
					p += _add_page(DFLAG_ISSET(dp,DEBUG),p,&pages[x]);
					buflen -= pages[x].size;
				}
			}
		} else
			return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	} else {
		/* Specific page */
		int found = 0;
		for(x=0; pages[x].data; x++) {
			dprintk("mode_sense: pages[%d].num: %x, page: %x\n", x, pages[x].num, page);
			if (pages[x].num == page) {
				p += _add_page(DFLAG_ISSET(dp,DEBUG),p,&pages[x]);
				found = 1;
				break;
			}
		}
		if (!found) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}

	*data_len = p - oldp;

	return GOOD;
}

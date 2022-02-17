/*
Status 		Sense Key 	Additional Sense Code
GOOD 		not applicable 	not applicable
CHECK CONDITION ILLEGAL REQUEST LOGICAL UNIT NOT SUPPORTED
CHECK CONDITION NOT READY 	LOGICAL UNIT DOES NOT RESPOND TO SELECTION
CHECK CONDITION NOT READY 	MEDIUM NOT PRESENT
CHECK CONDITION NOT READY	LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE
CHECK CONDITION NOT READY	LOGICAL UNIT IS IN PROCESS OF BECOMING READY
CHECK CONDITION NOT READY 	LOGICAL UNIT NOT READY, INITIALIZING COMMAND REQUIRED
CHECK CONDITION NOT READY 	LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED
CHECK CONDITION NOT READY	LOGICAL UNIT NOT READY, FORMAT IN PROGRESS

04h/00h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE
04h/01h  DTLPWROMAEBKVF  LOGICAL UNIT IS IN PROCESS OF BECOMING READY
04h/02h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, INITIALIZING COMMAND REQUIRED
04h/03h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED
04h/04h  DTL  RO   B     LOGICAL UNIT NOT READY, FORMAT IN PROGRESS

*/

#define CAUSE_NOT_REPORTABLE 0x00
#define INITIALIZING_COMMAND_REQUIRED 0x02
#define FORMAT_IN_PROGRESS 0x04

static int test_unit_ready(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rc;

	dprintk("test_unit_ready: open: %d\n", DFLAG_ISSET(dp, OPEN));
	if (DFLAG_ISSET(dp, OPEN)) {
		dprintk("test_unit_ready: format: %d\n", DFLAG_ISSET(dp, FORMAT));
		if (DFLAG_ISSET(dp, FORMAT))
			rc = check_condition(dp, NOT_READY, LOGICAL_UNIT_NOT_READY, FORMAT_IN_PROGRESS);
		else
			rc = GOOD;
	} else {
		dprintk("test_unit_ready: removable: %d\n", DFLAG_ISSET(dp, RMB));
		if (DFLAG_ISSET(dp, RMB)) {
//			rc = check_condition(dp, NOT_READY, MEDIUM_NOT_PRESENT, 0);
			rc = GOOD;
		}
		else
			rc = check_condition(dp, NOT_READY, LOGICAL_UNIT_NOT_READY, CAUSE_NOT_REPORTABLE);
	}

	return rc;
}

#if 0
static int unit_ready(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int error, rc;

	rc = GOOD;
	error = (wp->dp->os_handle == 0 ? host_open(wp->dp) : GOOD);
	if (error) {
		switch(wp->dp->type) {
		case TYPE_ROM:
		case TYPE_TAPE:
			rc = check_condition(wp->dp, NOT_READY, MEDIUM_NOT_PRESENT, 0x2);
			break;
		default:
			rc = check_condition(wp->dp, NOT_READY, LOGICAL_UNIT_NOT_READY, 0x2);
			break;
		}
	}

	return rc;
}
#endif

static int request_sense(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	memset(temp, 0, 18);
	if (scp->cmnd[1] & 1) {
		temp[0] = 0x72;
		temp[1] = dp->key;
		temp[2] = dp->asc;
		temp[3] = dp->asq;
	} else {
		temp[0] = 0x70;
		temp[2] = dp->key;
		temp[7] = 0xa;
		temp[12] = dp->asc;
		temp[13] = dp->asq;
	}
	return response(scp, &temp, min(scp->cmnd[4], 18));
}

static int format_unit(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int reassign_blocks(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


static int read_write_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = (*(p+1) & 0x1f) << 16 | *(p+2) << 8 | *(p+3);
	num = *(p+4) ? *(p+4) : 0xff;

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}

#define write_6 read_write_6
#if 0
static int write_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = (*(p+1) & 0x1f) << 16 | *(p+2) << 8 | *(p+3);
	num = *(p+4) ? *(p+4) : 0xff;

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

#define DEBUG_INQ 1

static int inquiry(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int x, alloc_len;

	alloc_len = (scp->cmnd[3] << 8) + scp->cmnd[4];
#if DEBUG_INQ
	if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: alloc_len: %d\n", alloc_len);
#endif

	/* EVPD? */
	if (scp->cmnd[1] & 1) {
		struct vscsi_rom_page *vpd = dp->rom->vpd;
		int page = scp->cmnd[2];

#if DEBUG_INQ
		if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: sending VPD page %d\n", page);
#endif
		for(x=0; vpd[x].data; x++) {
			if (vpd[x].num == page)
				return response(scp, vpd[x].data, min(alloc_len, vpd[x].size));
		}
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	/* Standard page */
	} else {
		/* If the EVPD bit is set to zero, the device server shall return the standard INQUIRY
		   data (see 6.4.2). If the PAGE CODE field is not set to zero when the EVPD bit is set
		   to zero, the command shall be terminated with CHECK CONDITION status, with the sense
		   key set to ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN CDB. */
		if (scp->cmnd[2] != 0) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

#if DEBUG_INQ
		if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: sending STD page\n");
#endif
#if 0
		{
			unsigned char *data;

			data = dp->rom->std.data;
			printk("INQUIRY: ADDR16=%d\n", (data[6] & 0x01) != 0);
			printk("INQUIRY: WBUS16=%d\n", (data[7] & 0x20) != 0);
			printk("INQUIRY: SYNC=%d\n", (data[7] & 0x10) != 0);
			printk("INQUIRY: CMDQUE=%d\n", (data[7] & 0x02) != 0);
		}
#endif

		return response(scp, dp->rom->std.data, min(alloc_len, dp->rom->std.size));
	}
}
static int mode_select_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


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

static int mode_sense_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int err,dl,buflen,data_len;

	/* DBD */
	dl = (scp->cmnd[1] & 0x08 ? 0 : 8);
        buflen = min(scp->cmnd[4],sizeof(temp));

	/* MEDIUM TYPE */
	temp[1] = 0;
	/* DEVICE-SPECIFIC PARAMETER */
	temp[2] = 0;
	/* BLOCK DESCRIPTOR LENGTH */
	temp[3] = dl;

	err =  mode_sense(dp, scp, &temp[4], buflen - 4, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* MODE DATA LENGTH */
	temp[0] = data_len;
	return response(scp, &temp, data_len);
}
static int start_stop_unit(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int start;

#if 0
0 OPERATION CODE (1Bh)
1 Reserved IMMED
2 Reserved
3 Reserved POWER CONDITION MODIFIER
4 POWER CONDITION Reserved NO_FLUSH LOEJ START
5 CONTROL
#endif
#if 0
POWER CONDITION field
0h START_VALID Process the START and LOEJ bits.
1h ACTIVE Place the device into the active power condition.
2h IDLE Place the device into the idle power condition.
3h STANDBY Place the device into the standby power condition.
4h Reserved Reserved
5h Obsolete Obsolete
6h Reserved Reserved
7h LU_CONTROL Transfer control of power conditions to the logical unit.
8h to 9h Reserved Reserved
Ah FORCE_IDLE_0 Force the idle condition timer to zero.
Bh FORCE_STANDBY_0 Force the standby condition timer to zero.
Ch to Fh Reserved Reserved
#endif
	start = scp->cmnd[4] & 1;
	dprintk("start: %d\n", start);
//	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	return GOOD;
}

static int receive_diagnostic_results(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int send_diagnostic(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int prevent_allow_medium_removal(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rc,prevent;

	prevent = scp->cmnd[4] & 1;
	dprintk("prevent: %d\n", prevent);
	rc = GOOD;

	/* Removable? */
	if (DFLAG_ISSET(dp, RMB)) {
		if (prevent) {
			/* Already open? */
			if (DFLAG_ISSET(dp,OPEN)) {
//				dp->flags |= DEVICE_FLAG_LOCKED;
				rc = 0;
			} else {
				/* Open it */
				if (vscsi_device_open(dp))
					rc = check_condition(dp, NOT_READY, MEDIUM_NOT_PRESENT, 0);
				else
					rc = 0;
			}
		} else {
			if (DFLAG_ISSET(dp,OPEN)) vscsi_device_close(dp);
			rc = 0; /* XXX */
		}
	} else
		rc = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	return rc;
}

static int read_capacity_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long max_lba;
#if VSCSI_STRICT
	unsigned long long lba;
	unsigned char *p = scp->cmnd;

	/* Get the lba requested */
	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);

	/* The LOGICAL BLOCK ADDRESS field shall be set to zero if the PMI bit is set to zero. If the PMI bit is set to zero and
the LOGICAL BLOCK ADDRESS field is not set to zero, then the device server shall terminate the command with
CHECK CONDITION status with the sense key set to ILLEGAL REQUEST and the additional sense code set
to INVALID FIELD IN CDB */
	if ((p[8] & 1) == 0 && lba != 0) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif

//	calc_max_lba(dp->fp->size, scp->device->sector_size, &max_lba);
	calc_max_lba(dp->fp->size, dp->rom->sector_size, &max_lba);
	if (max_lba > 0xfffffffe || scp->cmnd[8] & 1) {
		temp[0] = 0xff;
		temp[1] = 0xff;
		temp[2] = 0xff;
		temp[3] = 0xff;
	} else {
		temp[0] = (max_lba >> 24);
		temp[1] = (max_lba >> 16) & 0xff;
		temp[2] = (max_lba >> 8) & 0xff;
		temp[3] = max_lba & 0xff;
	}
#if 0
	temp[4] = (scp->device->sector_size >> 24);
	temp[5] = (scp->device->sector_size >> 16) & 0xff;
	temp[6] = (scp->device->sector_size >> 8) & 0xff;
	temp[7] = scp->device->sector_size & 0xff;
#endif
	temp[4] = (dp->rom->sector_size >> 24);
	temp[5] = (dp->rom->sector_size >> 16) & 0xff;
	temp[6] = (dp->rom->sector_size >> 8) & 0xff;
	temp[7] = dp->rom->sector_size & 0xff;

	return response(scp, &temp, 8);
}

static int read_write_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p  = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+7) << 8 | *(p+8);

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#define write_10 read_write_10
#if 0
static int write_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p  = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+7) << 8 | *(p+8);

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int write_and_verify_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int verify_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int pre_fetch_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int synchronize_cache_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	/* We have no cache atm */
	return GOOD;
}

static int read_defect_data_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


static unsigned char echobuf[4096];
static int echobuf_len = 0;

static int write_buffer(struct vscsi_device *dp, struct scsi_cmnd *scp) {
//	int mode, id, off, len;
	int mode,len;
#if 0
0 OPERATION CODE (3Bh)
1 Reserved MODE
2 BUFFER ID
3 (MSB) BUFFER OFFSET
5 (LSB)
6 (MSB) PARAMETER LIST LENGTH
8 (LSB)
9 CONTROL
#endif
	mode = scp->cmnd[1] & 0xF;
//	id = scp->cmnd[2];
//	off = scp->cmnd[3] << 16 | scp->cmnd[4] << 8 | scp->cmnd[5];
	len = scp->cmnd[6] << 16 | scp->cmnd[7] << 8 | scp->cmnd[8];
//	dprintk("write_buffer: mode: %x, id: %d, off: %x, len: %d\n", mode, id, off, len);

#if 0
6.39.9 Write data to echo buffer mode (0Ah)
In this mode the device server transfers data from the application client and stores it in an echo buffer. An echo
buffer is assigned in the same manner by the device server as it would for a write operation. Data shall be sent
aligned on four-byte boundaries.
The BUFFER ID and BUFFER OFFSET fields shall be ignored in this mode.
NOTE 42 - It is recommended that the logical unit assign echo buffers on a per I_T nexus basis to limit the number
of exception conditions that may occur when I_T nexuses are present.
Upon successful completion of a WRITE BUFFER command the data shall be preserved in the echo buffer unless
there is an intervening command to any logical unit in which case the data may be changed.
The PARAMETER LIST LENGTH field specifies the maximum number of bytes that shall be transferred from the
Data-Out Buffer to be stored in the echo buffer. The application client should ensure that the parameter list length
does not exceed the capacity of the echo buffer. The capacity of the echo buffer is indicated by the BUFFER
CAPACITY field in the READ BUFFER echo buffer descriptor (see 6.16.7). If the PARAMETER LIST LENGTH field
specifies a transfer in excess of the buffer capacity, the command shall be terminated with CHECK CONDITION
status, with the sense key set to ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN
CDB.
#endif

	/* We only support the echo buffer */
	dprintk("write_buffer: mode: %x, len: %d\n", mode, len);
	if (mode == 0x0A) {
		dprintk("write_buffer: len: %d, echobuf: %d\n", len, (int)sizeof(echobuf));
		if (len > sizeof(echobuf))
			return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
		echobuf_len = len;
		return xfer_to_dev(scp, echobuf, sizeof(echobuf));
	}

	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}
static int read_buffer(struct vscsi_device *dp, struct scsi_cmnd *scp) {
//	int mode, id, off, len;
	int mode, len;

#if 0
0 OPERATION CODE (3Ch)
1 Reserved MODE
2 BUFFER ID
3 (MSB) BUFFER OFFSET
5 (LSB)
6 (MSB) ALLOCATION LENGTH
8 (LSB)
9 CONTROL
#endif
	mode = scp->cmnd[1] & 0x0F;
//	id = scp->cmnd[2];
//	off = scp->cmnd[3] << 16 | scp->cmnd[4] << 8 | scp->cmnd[5];
	len = scp->cmnd[6] << 16 | scp->cmnd[7] << 8 | scp->cmnd[8];
//	dprintk("read_buffer: mode: %x, id: %d, off: %d, len: %d\n", mode, id, off, len);

#if 0
6.16.6 Read data from echo buffer mode (0Ah)
In this mode the device server transfers data to the application client from the echo buffer that was written by the
most recent WRITE BUFFER command wit the MODE field set to write data to echo buffer (see 6.39.9) received on
the same I_T nexus. The READ BUFFER command shall return the same number of bytes of data as received in
the prior WRITE BUFFER command with the mode field set to write data to echo buffer, limited by the allocation
length as described in 4.3.5.6.
The BUFFER ID and BUFFER OFFSET fields are ignored in this mode.

If no WRITE BUFFER command with the mode set to write data to echo buffer received on this I_T nexus has
completed without an error, then the READ BUFFER command shall terminate with CHECK CONDITION status,
with the sense key set to ILLEGAL REQUEST, and the additional sense code set to COMMAND SEQUENCE
ERROR. If the data in the echo buffer has been overwritten by another I_T nexus, the READ BUFFER command
shall be terminated with CHECK CONDITION status, with the sense key set to ABORTED COMMAND, and the
additional sense code set to ECHO BUFFER OVERWRITTEN.
After a WRITE BUFFER command with the mode set to write data to echo buffer has completed without an error,
the application client may send multiple READ BUFFER commands with the mode set to read data from echo
buffer in order to read the echo buffer data multiple times.

6.16.7 Echo buffer descriptor mode (0Bh)
In this mode, a maximum of four bytes of READ BUFFER descriptor information is returned. The device server
shall return the descriptor information for the echo buffer. If there is no echo buffer implemented, the device server
shall return all zeros in the READ BUFFER descriptor. The BUFFER ID field and BUFFER OFFSET field are reserved in
this mode. The allocation length should be set to four or greater. The READ BUFFER descriptor is defined as
shown in table 186.

0 Reserved EBOS
1 Reserved
2 Reserved (MSB)
3 BUFFER CAPACITY (LSB)

The BUFFER CAPACITY field shall return the size of the echo buffer in bytes aligned to a four-byte boundary. The
maximum echo buffer size is 4 096 bytes.
If the echo buffer is implemented, the echo buffer descriptor shall be implemented.
An echo buffer overwritten supported (EBOS) bit set to one indicates either:
a) The device server returns the ECHO BUFFER OVERWRITTEN additional sense code if the data being
read from the echo buffer is not the data previously written by the same I_T nexus, or
b) The device server ensures echo buffer data returned to each I_T nexus is the same as that previously
written by that I_T nexus.
An EBOS bit set to zero specifies that the echo buffer may be overwritten by any intervening command received on
any I_T nexus.
A READ BUFFER command with the mode set to echo buffer descriptor may be used to determine the echo buffer
capacity and supported features before a WRITE BUFFER command with the mode set to write data to echo buffer
(see 6.39.9) is sent.
#endif

	/* Echo buffer descriptor mode */
	dprintk("read_buffer: mode: %x, len: %d\n", mode, len);
	if (mode == 0x0A) {
		return response(scp, &echobuf, echobuf_len);
	} else if (mode == 0x0B) {
		int n = sizeof(echobuf);
		temp[0] = 0;
		temp[1] = 0;
		temp[2] = n >> 8;
		temp[4] = n & 0xFF;
		return response(scp, &temp, 4);
	}

	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_long_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_long_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_same_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int vscsi_unmap(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int log_select(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int log_sense(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int xdwrite_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int xpwrite_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int xdread_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int xdwriteread_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int mode_select_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


static int mode_sense_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int err,alloc_len,buflen,data_len;
	int dl, dbd, llba;
/*
If the Long LBA Accepted (LLBAA) bit is set to one, the device server is allowed to return parameter data with the
LONGLBA bit equal to one (see 7.4.3). If LLBAA bit is set to zero, the LONGLBA bit shall be zero in the parameter data
returned by the device server

If the Long LBA (LONGLBA) bit is set to zero, the mode parameter block descriptor(s), if any, are each eight bytes
long and have the format described in 7.4.4.1. If the LONGLBA bit is set to one, the mode parameter block
descriptor(s), if any, are each sixteen bytes long and have a format described in a command standard (see 3.1.27).

The BLOCK DESCRIPTOR LENGTH field contains the length in bytes of all the block descriptors. It is equal to the
number of block descriptors times eight if the LONGLBA bit is set to zero or times sixteen if the LONGLBA bit is set to
one, and does not include mode pages or vendor specific parameters (e.g., page code set to zero), if any, that may
follow the last block descriptor. A block descriptor length of zero indicates that no block descriptors are included in
the mode parameter list. This condition shall not be considered an error.
*/

	dbd = ((scp->cmnd[1] & 0x08) != 0);
	llba = ((scp->cmnd[1] & 0x10) != 0);
	dprintk("dbd: %d, llba: %d\n", dbd, llba);

/*
	Mode parameter header
	Block descriptor(s)
	Mode page(s)
*/

	alloc_len = scp->cmnd[7] << 8 | scp->cmnd[8];
	buflen = min(alloc_len,sizeof(temp));
	dprintk("alloc_len: %d, buflen: %d\n", alloc_len, buflen);

	/* Desc length */
	dl = (dbd ? 0 : (llba ? 16 : 8));

	/* Header */
	/* 2 MEDIUM TYPE */
	temp[2] = 0;
	/* 3 DEVICE-SPECIFIC PARAMETER */
	temp[3] = 0;
	/* 4 Reserved LONGLBA */
	temp[4] = llba;
	/* 5 Reserved */
	temp[5] = 0;
	/* 6-7 BLOCK DESCRIPTOR LENGTH */
	temp[6] = 0;
	temp[7] = dl;

	err =  mode_sense(dp, scp, &temp[8], buflen - 8, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* 0-1 MODE DATA LENGTH */
	temp[0] = (data_len >> 8) & 0xff;
	temp[1] = data_len & 0xff;

//	scp->device->host->max_cmd_len = 0;
	return response(scp, &temp, data_len);

//	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#if 0

	/* 3 DEVICE-SPECIFIC PARAMETER */
	switch(dp->type) {
	case 0:
		/* WP Reserved DPOFUA Reserved */
		header[3] = 0;
		break;
	default:
		break;
	}
	/* 4 Reserved LONGLBA */
	header[4] = 0;

	int err,dl,alloc_len,buflen,data_len;

	/* DBD */
	if (scp->cmnd[1] & 0x08) {
		dl = 0;
	} else {
		/* LLBA */
		if (scp->cmnd[1] & 0x10)
			dl = 16;
		else
			dl = 8;
	}

	alloc_len = scp->cmnd[7] << 8 | scp->cmnd[8];
	buflen = min(alloc_len,sizeof(temp));
	dprintk("alloc_len: %d, buflen: %d\n", alloc_len, buflen);

	err =  mode_sense(dp, scp, &temp[8], buflen - 8, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* 0-1 MODE DATA LENGTH */
	temp[0] = (data_len >> 8) & 0xff;
	temp[1] = data_len & 0xff;
	return response(scp, &temp, data_len);
#endif
}
static int persistent_reserve_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int persistent_reserve_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int extended_cdb(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int variable_length_cdb(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int extended_copy(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int receive_copy_results(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int ata_command_pass_through_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int access_control_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int access_control_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


static int read_write_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;
#if 0
	register int x;

	lba = 0;
	for (x = 0; x < 8; x++) {
		if (x) lba <<= 8;
		lba |= *(p+2+x);
	}
#endif
	lba =  ((u64)*(p+2)) << 56 | ((u64)*(p+3)) << 48 | ((u64)*(p+4)) << 40 | ((u64)*(p+5)) << 32 | \
		((u64)*(p+6)) << 24 | ((u64)*(p+7)) << 16 | ((u64)*(p+8)) << 8 | ((u64)*(p+9));
	num = *(p+10) << 24 | *(p+11) << 16 | *(p+12) << 8 | *(p+13);

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
static int compare_and_write(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

#define write_16 read_write_16
#if 0
static int write_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;
	register int x;

	lba = 0;
	for (x = 0; x < 8; x++) {
		if (x) lba <<= 8;
		lba |= *(p+2+x);
	}
	num = *(p+10) << 24 | *(p+11) << 16 | *(p+12) << 8 | *(p+13);

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int orwrite(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_attribute(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_attribute(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_and_verify_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int verify_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int pre_fetch_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int synchronize_cache_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	/* We have no cache ATM */
	return GOOD;
}

static int write_same_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int report_luns(struct vscsi_device *dp, struct scsi_cmnd *scp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	unsigned char *p = scp->cmnd, *lp;
	int len;
	int i,t,a,count;
	struct vscsi_device *ldp;
	struct scsi_lun lun;

	/* NOTE 38 - Device servers compliant with SPC return CHECK CONDITION status, with the sense key set to
	   ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN CDB when the allocation length is
	   less than 16 bytes. */
	len = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);
	if (len < 16) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	/* Get a count of the luns */
	count = 0;
	for(i=0; i < VSCSI_NUM_LUNS; i++) {
		ldp = vscsi_get_device(dp->chan, dp->id, i);
		if (!DFLAG_ISSET(ldp,INUSE)) break;
		count++;
	}
	dprintk("count: %d\n", count);

	/*  If the device server is not ready with the logical unit inventory or if the inventory list is null for the requesting I_T nexus and the SELECT REPORT field set to 02h, then the device server shall provide a default logical unit inventory that contains at least LUN 0 or the REPORT LUNS well known logical unit (see 8.2). A non-empty peripheral device logical unit inventory that does not contain either LUN 0 or the REPORT LUNS well known logical unit is valid. */
	memset(temp, 0, 16);
	t = 16;
	a = *(p+2);
	dprintk("a: %d\n", a);
	switch(a) {
	case 0: /* Addressing */
		dprintk("Addressing...\n");
	case 2: /* All - only 1 lun (0) supported */
		dprintk("All...\n");
		len = count * 8;
		temp[0] = (len >> 24) & 0xFF;
		temp[1] = (len >> 16) & 0xFF;
		temp[2] = (len >> 8) & 0xFF;
		temp[3] = len & 0xFF;
		t = 8;
		for(i=0; i < count; i++) {
			int_to_scsilun(i, &lun);
			lp = (unsigned char *) &lun;
			temp[t++] = lp[0];
			temp[t++] = lp[1];
			temp[t++] = lp[2];
			temp[t++] = lp[3];
			temp[t++] = lp[4];
			temp[t++] = lp[5];
			temp[t++] = lp[6];
			temp[t++] = lp[7];
		}
		dprintk("t: %d\n", t);
		break;
	case 1: /* Well-known (not supported) */
	default:
		t = 16;
		break;
	}
	dprintk("t: %d\n", t);
	scp->result = response(scp, &temp, t);

	return 0;
#else
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif
}
static int ata_command_pass_through_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
        return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int security_protocol_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int maintenance_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int maintenance_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


static int read_write_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#define write_12 read_write_12
#if 0
static int write_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int write_and_verify_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int verify_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int security_protocol_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_defect_data_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int redundancy_group_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int redundancy_group_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int spare_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int spare_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int volume_set_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int volume_set_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


static int read_capacity_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long max_lba;
#if VSCSI_STRICT
	unsigned long long lba;
	unsigned char *p = scp->cmnd;

	/* Get the lba requested */
	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);

	/* The LOGICAL BLOCK ADDRESS field shall be set to zero if the PMI bit is set to zero. If the PMI bit is set to zero and
the LOGICAL BLOCK ADDRESS field is not set to zero, then the device server shall terminate the command with
CHECK CONDITION status with the sense key set to ILLEGAL REQUEST and the additional sense code set
to INVALID FIELD IN CDB */
	if ((p[8] & 1) == 0 && lba != 0) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif

	calc_max_lba(dp->fp->size, dp->rom->sector_size, &max_lba);
	memset(temp,0,32);
	if (max_lba > 0xFFFFFFFFFFFFFFFELL || scp->cmnd[14] & 1) {
		temp[0] = 0xff;
		temp[1] = 0xff;
		temp[2] = 0xff;
		temp[3] = 0xff;
		temp[4] = 0xff;
		temp[5] = 0xff;
		temp[6] = 0xff;
		temp[7] = 0xff;
	} else {
		temp[0] = (max_lba >> 56);
		temp[1] = (max_lba >> 48) & 0xff;
		temp[2] = (max_lba >> 40) & 0xff;
		temp[3] = (max_lba >> 32) & 0xff;
		temp[4] = (max_lba >> 24) & 0xff;
		temp[5] = (max_lba >> 16) & 0xff;
		temp[6] = (max_lba >> 8) & 0xff;
		temp[7] = max_lba & 0xff;
	}

	temp[8] = (dp->rom->sector_size >> 24);
	temp[9] = (dp->rom->sector_size >> 16) & 0xff;
	temp[10] = (dp->rom->sector_size >> 8) & 0xff;
	temp[11] = dp->rom->sector_size & 0xff;

	return response(scp, &temp, 32);
}

static vscsi_ops_t vscsi_disk_ops[256] = {
	/* 00 */ test_unit_ready,
	/* 01 */ 0,
	/* 02 */ 0,
	/* 03 */ request_sense,
	/* 04 */ format_unit,
	/* 05 */ 0,
	/* 06 */ 0,
	/* 07 */ reassign_blocks,
	/* 08 */ read_write_6,
	/* 09 */ 0,
	/* 0a */ write_6,
	/* 0b */ 0,
	/* 0c */ 0,
	/* 0d */ 0,
	/* 0e */ 0,
	/* 0f */ 0,
	/* 10 */ 0,
	/* 11 */ 0,
	/* 12 */ inquiry,
	/* 13 */ 0,
	/* 14 */ 0,
	/* 15 */ mode_select_6,
	/* 16 */ 0,
	/* 17 */ 0,
	/* 18 */ 0,
	/* 19 */ 0,
	/* 1a */ mode_sense_6,
	/* 1b */ start_stop_unit,
	/* 1c */ receive_diagnostic_results,
	/* 1d */ send_diagnostic,
	/* 1e */ prevent_allow_medium_removal,
	/* 1f */ 0,
	/* 20 */ 0,
	/* 21 */ 0,
	/* 22 */ 0,
	/* 23 */ 0,
	/* 24 */ 0,
	/* 25 */ read_capacity_10,
	/* 26 */ 0,
	/* 27 */ 0,
	/* 28 */ read_write_10,
	/* 29 */ 0,
	/* 2a */ write_10,
	/* 2b */ 0,
	/* 2c */ 0,
	/* 2d */ 0,
	/* 2e */ write_and_verify_10,
	/* 2f */ verify_10,
	/* 30 */ 0,
	/* 31 */ 0,
	/* 32 */ 0,
	/* 33 */ 0,
	/* 34 */ pre_fetch_10,
	/* 35 */ synchronize_cache_10,
	/* 36 */ 0,
	/* 37 */ read_defect_data_10,
	/* 38 */ 0,
	/* 39 */ 0,
	/* 3a */ 0,
	/* 3b */ write_buffer,
	/* 3c */ read_buffer,
	/* 3d */ 0,
	/* 3e */ read_long_10,
	/* 3f */ write_long_10,
	/* 40 */ 0,
	/* 41 */ write_same_10,
	/* 42 */ vscsi_unmap,
	/* 43 */ 0,
	/* 44 */ 0,
	/* 45 */ 0,
	/* 46 */ 0,
	/* 47 */ 0,
	/* 48 */ 0,
	/* 49 */ 0,
	/* 4a */ 0,
	/* 4b */ 0,
	/* 4c */ log_select,
	/* 4d */ log_sense,
	/* 4e */ 0,
	/* 4f */ 0,
	/* 50 */ xdwrite_10,
	/* 51 */ xpwrite_10,
	/* 52 */ xdread_10,
	/* 53 */ xdwriteread_10,
	/* 54 */ 0,
	/* 55 */ mode_select_10,
	/* 56 */ 0,
	/* 57 */ 0,
	/* 58 */ 0,
	/* 59 */ 0,
	/* 5a */ mode_sense_10,
	/* 5b */ 0,
	/* 5c */ 0,
	/* 5d */ 0,
	/* 5e */ persistent_reserve_in,
	/* 5f */ persistent_reserve_out,
	/* 60 */ 0,
	/* 61 */ 0,
	/* 62 */ 0,
	/* 63 */ 0,
	/* 64 */ 0,
	/* 65 */ 0,
	/* 66 */ 0,
	/* 67 */ 0,
	/* 68 */ 0,
	/* 69 */ 0,
	/* 6a */ 0,
	/* 6b */ 0,
	/* 6c */ 0,
	/* 6d */ 0,
	/* 6e */ 0,
	/* 6f */ 0,
	/* 70 */ 0,
	/* 71 */ 0,
	/* 72 */ 0,
	/* 73 */ 0,
	/* 74 */ 0,
	/* 75 */ 0,
	/* 76 */ 0,
	/* 77 */ 0,
	/* 78 */ 0,
	/* 79 */ 0,
	/* 7a */ 0,
	/* 7b */ 0,
	/* 7c */ 0,
	/* 7d */ 0,
	/* 7e */ extended_cdb,
	/* 7f */ variable_length_cdb,
	/* 80 */ 0,
	/* 81 */ 0,
	/* 82 */ 0,
	/* 83 */ extended_copy,
	/* 84 */ receive_copy_results,
	/* 85 */ ata_command_pass_through_16,
	/* 86 */ access_control_in,
	/* 87 */ access_control_out,
	/* 88 */ read_write_16,
	/* 89 */ compare_and_write,
	/* 8a */ write_16,
	/* 8b */ orwrite,
	/* 8c */ read_attribute,
	/* 8d */ write_attribute,
	/* 8e */ write_and_verify_16,
	/* 8f */ verify_16,
	/* 90 */ pre_fetch_16,
	/* 91 */ synchronize_cache_16,
	/* 92 */ 0,
	/* 93 */ write_same_16,
	/* 94 */ 0,
	/* 95 */ 0,
	/* 96 */ 0,
	/* 97 */ 0,
	/* 98 */ 0,
	/* 99 */ 0,
	/* 9a */ 0,
	/* 9b */ 0,
	/* 9c */ 0,
	/* 9d */ 0,
	/* 9e */ read_capacity_16,
	/* 9f */ 0,
	/* a0 */ report_luns,
	/* a1 */ ata_command_pass_through_12,
	/* a2 */ security_protocol_in,
	/* a3 */ maintenance_in,
	/* a4 */ maintenance_out,
	/* a5 */ 0,
	/* a6 */ 0,
	/* a7 */ 0,
	/* a8 */ read_write_12,
	/* a9 */ 0,
	/* aa */ write_12,
	/* ab */ 0,
	/* ac */ 0,
	/* ad */ 0,
	/* ae */ write_and_verify_12,
	/* af */ verify_12,
	/* b0 */ 0,
	/* b1 */ 0,
	/* b2 */ 0,
	/* b3 */ 0,
	/* b4 */ 0,
	/* b5 */ security_protocol_out,
	/* b6 */ 0,
	/* b7 */ read_defect_data_12,
	/* b8 */ 0,
	/* b9 */ 0,
	/* ba */ redundancy_group_in,
	/* bb */ redundancy_group_out,
	/* bc */ spare_in,
	/* bd */ spare_out,
	/* be */ volume_set_in,
	/* bf */ volume_set_out,
	/* c0 */ 0,
	/* c1 */ 0,
	/* c2 */ 0,
	/* c3 */ 0,
	/* c4 */ 0,
	/* c5 */ 0,
	/* c6 */ 0,
	/* c7 */ 0,
	/* c8 */ 0,
	/* c9 */ 0,
	/* ca */ 0,
	/* cb */ 0,
	/* cc */ 0,
	/* cd */ 0,
	/* ce */ 0,
	/* cf */ 0,
	/* d0 */ 0,
	/* d1 */ 0,
	/* d2 */ 0,
	/* d3 */ 0,
	/* d4 */ 0,
	/* d5 */ 0,
	/* d6 */ 0,
	/* d7 */ 0,
	/* d8 */ 0,
	/* d9 */ 0,
	/* da */ 0,
	/* db */ 0,
	/* dc */ 0,
	/* dd */ 0,
	/* de */ 0,
	/* df */ 0,
	/* e0 */ 0,
	/* e1 */ 0,
	/* e2 */ 0,
	/* e3 */ 0,
	/* e4 */ 0,
	/* e5 */ 0,
	/* e6 */ 0,
	/* e7 */ 0,
	/* e8 */ 0,
	/* e9 */ 0,
	/* ea */ 0,
	/* eb */ 0,
	/* ec */ 0,
	/* ed */ 0,
	/* ee */ 0,
	/* ef */ 0,
	/* f0 */ 0,
	/* f1 */ 0,
	/* f2 */ 0,
	/* f3 */ 0,
	/* f4 */ 0,
	/* f5 */ 0,
	/* f6 */ 0,
	/* f7 */ 0,
	/* f8 */ 0,
	/* f9 */ 0,
	/* fa */ 0,
	/* fb */ 0,
	/* fc */ 0,
	/* fd */ 0,
	/* fe */ 0,
	/* ff */ 0,
};

static int vscsi_call_op(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int r;
	if (dp->ops[scp->cmnd[0]]) {
		r = dp->ops[scp->cmnd[0]](dp, scp);
	} else {
		r = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}
	return r;
}

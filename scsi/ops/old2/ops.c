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
static int send_diagnostic(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
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
static int report_luns(struct vscsi_device *dp, struct scsi_cmnd *scp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	unsigned char *p = scp->cmnd, *lp;
	int len;
	int i,t,count;
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
	dprintk("report_luns: count: %d\n", count);

	/*  If the device server is not ready with the logical unit inventory or if the inventory list is null for the requesting I_T nexus and the SELECT REPORT field set to 02h, then the device server shall provide a default logical unit inventory that contains at least LUN 0 or the REPORT LUNS well known logical unit (see 8.2). A non-empty peripheral device logical unit inventory that does not contain either LUN 0 or the REPORT LUNS well known logical unit is valid. */
	memset(temp, 0, 16);
	t = 16;
	switch(*(p+2)) {
	case 0: /* Addressing */
	case 2: /* All - only 1 lun (0) supported */
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
		break;
	case 1: /* Well-known (not supported) */
	default:
		t = 16;
		break;
	}
	dprintk("report_luns: t: %d\n", t);
	scp->result = response(scp, &temp, t);

	return 0;
#else
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif
}

static int read_capacity_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


typedef int (*vscsi_opfunc_t)(struct vscsi_device *, struct scsi_cmnd *);

static vscsi_opfunc_t vscsi_disk_ops[256] = {
	/* 00 */ test_unit_ready,
	/* 01 */ 0,
	/* 02 */ 0,
	/* 03 */ request_sense,
	/* 04 */ format_unit,
	/* 05 */ 0,
	/* 06 */ 0,
	/* 07 */ 0,
	/* 08 */ read_write_6,
	/* 09 */ 0,
	/* 0a */ 0,
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
	/* 15 */ 0,
	/* 16 */ 0,
	/* 17 */ 0,
	/* 18 */ 0,
	/* 19 */ 0,
	/* 1a */ 0,
	/* 1b */ 0,
	/* 1c */ 0,
	/* 1d */ send_diagnostic,
	/* 1e */ 0,
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
	/* 2a */ 0,
	/* 2b */ 0,
	/* 2c */ 0,
	/* 2d */ 0,
	/* 2e */ 0,
	/* 2f */ 0,
	/* 30 */ 0,
	/* 31 */ 0,
	/* 32 */ 0,
	/* 33 */ 0,
	/* 34 */ 0,
	/* 35 */ 0,
	/* 36 */ 0,
	/* 37 */ 0,
	/* 38 */ 0,
	/* 39 */ 0,
	/* 3a */ 0,
	/* 3b */ 0,
	/* 3c */ 0,
	/* 3d */ 0,
	/* 3e */ 0,
	/* 3f */ 0,
	/* 40 */ 0,
	/* 41 */ 0,
	/* 42 */ 0,
	/* 43 */ 0,
	/* 44 */ 0,
	/* 45 */ 0,
	/* 46 */ 0,
	/* 47 */ 0,
	/* 48 */ 0,
	/* 49 */ 0,
	/* 4a */ 0,
	/* 4b */ 0,
	/* 4c */ 0,
	/* 4d */ 0,
	/* 4e */ 0,
	/* 4f */ 0,
	/* 50 */ 0,
	/* 51 */ 0,
	/* 52 */ 0,
	/* 53 */ 0,
	/* 54 */ 0,
	/* 55 */ 0,
	/* 56 */ 0,
	/* 57 */ 0,
	/* 58 */ 0,
	/* 59 */ 0,
	/* 5a */ 0,
	/* 5b */ 0,
	/* 5c */ 0,
	/* 5d */ 0,
	/* 5e */ 0,
	/* 5f */ 0,
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
	/* 7e */ 0,
	/* 7f */ 0,
	/* 80 */ 0,
	/* 81 */ 0,
	/* 82 */ 0,
	/* 83 */ 0,
	/* 84 */ 0,
	/* 85 */ 0,
	/* 86 */ 0,
	/* 87 */ 0,
	/* 88 */ read_write_16,
	/* 89 */ 0,
	/* 8a */ 0,
	/* 8b */ 0,
	/* 8c */ 0,
	/* 8d */ 0,
	/* 8e */ 0,
	/* 8f */ 0,
	/* 90 */ 0,
	/* 91 */ 0,
	/* 92 */ 0,
	/* 93 */ 0,
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
	/* a1 */ 0,
	/* a2 */ 0,
	/* a3 */ 0,
	/* a4 */ 0,
	/* a5 */ 0,
	/* a6 */ 0,
	/* a7 */ 0,
	/* a8 */ 0,
	/* a9 */ 0,
	/* aa */ 0,
	/* ab */ 0,
	/* ac */ 0,
	/* ad */ 0,
	/* ae */ 0,
	/* af */ 0,
	/* b0 */ 0,
	/* b1 */ 0,
	/* b2 */ 0,
	/* b3 */ 0,
	/* b4 */ 0,
	/* b5 */ 0,
	/* b6 */ 0,
	/* b7 */ 0,
	/* b8 */ 0,
	/* b9 */ 0,
	/* ba */ 0,
	/* bb */ 0,
	/* bc */ 0,
	/* bd */ 0,
	/* be */ 0,
	/* bf */ 0,
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
	vscsi_opfunc_t *ops = dp->ops;

	if (ops[scp->cmnd[0]]) {
		r = ops[scp->cmnd[0]](dp, scp);
	} else {
		r = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}
	return r;
}

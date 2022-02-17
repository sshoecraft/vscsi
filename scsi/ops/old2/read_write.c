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

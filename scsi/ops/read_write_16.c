
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

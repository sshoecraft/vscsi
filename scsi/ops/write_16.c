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


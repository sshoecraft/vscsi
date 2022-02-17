
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

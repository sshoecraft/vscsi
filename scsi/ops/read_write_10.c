
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

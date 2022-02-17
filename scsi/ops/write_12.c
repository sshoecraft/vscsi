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


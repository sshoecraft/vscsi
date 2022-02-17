
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

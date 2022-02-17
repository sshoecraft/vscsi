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

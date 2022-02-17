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


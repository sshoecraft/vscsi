static int read_cd_msf(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


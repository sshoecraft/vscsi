
static int vscsi_pass(struct vscsi_device *dp, struct scsi_cmnd *scp) {

	if (vscsi_file_pass(dp->fp, scp)) 
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	return 0;
}

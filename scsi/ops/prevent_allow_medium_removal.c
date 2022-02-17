static int prevent_allow_medium_removal(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rc,prevent;

	prevent = scp->cmnd[4] & 1;
	dprintk("prevent: %d\n", prevent);
	rc = GOOD;

	/* Removable? */
	if (DFLAG_ISSET(dp, RMB)) {
		if (prevent) {
			/* Already open? */
			if (DFLAG_ISSET(dp,OPEN)) {
//				dp->flags |= DEVICE_FLAG_LOCKED;
				rc = 0;
			} else {
				/* Open it */
				if (vscsi_device_open(dp))
					rc = check_condition(dp, NOT_READY, MEDIUM_NOT_PRESENT, 0);
				else
					rc = 0;
			}
		} else {
			if (DFLAG_ISSET(dp,OPEN)) vscsi_device_close(dp);
			rc = 0; /* XXX */
		}
	} else
		rc = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	return rc;
}


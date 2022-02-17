static int report_luns(struct vscsi_device *dp, struct scsi_cmnd *scp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	unsigned char *p = scp->cmnd, *lp;
	int len;
	int i,t,a,count;
	struct vscsi_device *ldp;
	struct scsi_lun lun;

	/* NOTE 38 - Device servers compliant with SPC return CHECK CONDITION status, with the sense key set to
	   ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN CDB when the allocation length is
	   less than 16 bytes. */
	len = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);
	if (len < 16) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	/* Get a count of the luns */
	count = 0;
	for(i=0; i < VSCSI_NUM_LUNS; i++) {
		ldp = vscsi_get_device(dp->chan, dp->id, i);
		if (!DFLAG_ISSET(ldp,INUSE)) break;
		count++;
	}
	dprintk("count: %d\n", count);

	/*  If the device server is not ready with the logical unit inventory or if the inventory list is null for the requesting I_T nexus and the SELECT REPORT field set to 02h, then the device server shall provide a default logical unit inventory that contains at least LUN 0 or the REPORT LUNS well known logical unit (see 8.2). A non-empty peripheral device logical unit inventory that does not contain either LUN 0 or the REPORT LUNS well known logical unit is valid. */
	memset(temp, 0, 16);
	t = 16;
	a = *(p+2);
	dprintk("a: %d\n", a);
	switch(a) {
	case 0: /* Addressing */
		dprintk("Addressing...\n");
	case 2: /* All - only 1 lun (0) supported */
		dprintk("All...\n");
		len = count * 8;
		temp[0] = (len >> 24) & 0xFF;
		temp[1] = (len >> 16) & 0xFF;
		temp[2] = (len >> 8) & 0xFF;
		temp[3] = len & 0xFF;
		t = 8;
		for(i=0; i < count; i++) {
			int_to_scsilun(i, &lun);
			lp = (unsigned char *) &lun;
			temp[t++] = lp[0];
			temp[t++] = lp[1];
			temp[t++] = lp[2];
			temp[t++] = lp[3];
			temp[t++] = lp[4];
			temp[t++] = lp[5];
			temp[t++] = lp[6];
			temp[t++] = lp[7];
		}
		dprintk("t: %d\n", t);
		break;
	case 1: /* Well-known (not supported) */
	default:
		t = 16;
		break;
	}
	dprintk("t: %d\n", t);
	scp->result = response(scp, &temp, t);

	return 0;
#else
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif
}

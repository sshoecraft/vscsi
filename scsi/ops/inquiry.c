
#define DEBUG_INQ 1

static int inquiry(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int x, alloc_len;

	alloc_len = (scp->cmnd[3] << 8) + scp->cmnd[4];
#if DEBUG_INQ
	if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: alloc_len: %d\n", alloc_len);
#endif

	/* EVPD? */
	if (scp->cmnd[1] & 1) {
		struct vscsi_rom_page *vpd = dp->rom->vpd;
		int page = scp->cmnd[2];

#if DEBUG_INQ
		if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: sending VPD page %d\n", page);
#endif
		for(x=0; vpd[x].data; x++) {
			if (vpd[x].num == page)
				return response(scp, vpd[x].data, min(alloc_len, vpd[x].size));
		}
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	/* Standard page */
	} else {
		/* If the EVPD bit is set to zero, the device server shall return the standard INQUIRY
		   data (see 6.4.2). If the PAGE CODE field is not set to zero when the EVPD bit is set
		   to zero, the command shall be terminated with CHECK CONDITION status, with the sense
		   key set to ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN CDB. */
		if (scp->cmnd[2] != 0) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

#if DEBUG_INQ
		if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: sending STD page\n");
#endif
#if 0
		{
			unsigned char *data;

			data = dp->rom->std.data;
			printk("INQUIRY: ADDR16=%d\n", (data[6] & 0x01) != 0);
			printk("INQUIRY: WBUS16=%d\n", (data[7] & 0x20) != 0);
			printk("INQUIRY: SYNC=%d\n", (data[7] & 0x10) != 0);
			printk("INQUIRY: CMDQUE=%d\n", (data[7] & 0x02) != 0);
		}
#endif

		return response(scp, dp->rom->std.data, min(alloc_len, dp->rom->std.size));
	}
}

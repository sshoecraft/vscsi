
static int mode_sense_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int err,alloc_len,buflen,data_len;
	int dl, dbd, llba;
/*
If the Long LBA Accepted (LLBAA) bit is set to one, the device server is allowed to return parameter data with the
LONGLBA bit equal to one (see 7.4.3). If LLBAA bit is set to zero, the LONGLBA bit shall be zero in the parameter data
returned by the device server

If the Long LBA (LONGLBA) bit is set to zero, the mode parameter block descriptor(s), if any, are each eight bytes
long and have the format described in 7.4.4.1. If the LONGLBA bit is set to one, the mode parameter block
descriptor(s), if any, are each sixteen bytes long and have a format described in a command standard (see 3.1.27).

The BLOCK DESCRIPTOR LENGTH field contains the length in bytes of all the block descriptors. It is equal to the
number of block descriptors times eight if the LONGLBA bit is set to zero or times sixteen if the LONGLBA bit is set to
one, and does not include mode pages or vendor specific parameters (e.g., page code set to zero), if any, that may
follow the last block descriptor. A block descriptor length of zero indicates that no block descriptors are included in
the mode parameter list. This condition shall not be considered an error.
*/

	dbd = ((scp->cmnd[1] & 0x08) != 0);
	llba = ((scp->cmnd[1] & 0x10) != 0);
	dprintk("dbd: %d, llba: %d\n", dbd, llba);

/*
	Mode parameter header
	Block descriptor(s)
	Mode page(s)
*/

	alloc_len = scp->cmnd[7] << 8 | scp->cmnd[8];
	buflen = min(alloc_len,sizeof(temp));
	dprintk("alloc_len: %d, buflen: %d\n", alloc_len, buflen);

	/* Desc length */
	dl = (dbd ? 0 : (llba ? 16 : 8));

	/* Header */
	/* 2 MEDIUM TYPE */
	temp[2] = 0;
	/* 3 DEVICE-SPECIFIC PARAMETER */
	temp[3] = 0;
	/* 4 Reserved LONGLBA */
	temp[4] = llba;
	/* 5 Reserved */
	temp[5] = 0;
	/* 6-7 BLOCK DESCRIPTOR LENGTH */
	temp[6] = 0;
	temp[7] = dl;

	err =  mode_sense(dp, scp, &temp[8], buflen - 8, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* 0-1 MODE DATA LENGTH */
	temp[0] = (data_len >> 8) & 0xff;
	temp[1] = data_len & 0xff;

//	scp->device->host->max_cmd_len = 0;
	return response(scp, &temp, data_len);

//	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#if 0

	/* 3 DEVICE-SPECIFIC PARAMETER */
	switch(dp->type) {
	case 0:
		/* WP Reserved DPOFUA Reserved */
		header[3] = 0;
		break;
	default:
		break;
	}
	/* 4 Reserved LONGLBA */
	header[4] = 0;

	int err,dl,alloc_len,buflen,data_len;

	/* DBD */
	if (scp->cmnd[1] & 0x08) {
		dl = 0;
	} else {
		/* LLBA */
		if (scp->cmnd[1] & 0x10)
			dl = 16;
		else
			dl = 8;
	}

	alloc_len = scp->cmnd[7] << 8 | scp->cmnd[8];
	buflen = min(alloc_len,sizeof(temp));
	dprintk("alloc_len: %d, buflen: %d\n", alloc_len, buflen);

	err =  mode_sense(dp, scp, &temp[8], buflen - 8, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* 0-1 MODE DATA LENGTH */
	temp[0] = (data_len >> 8) & 0xff;
	temp[1] = data_len & 0xff;
	return response(scp, &temp, data_len);
#endif
}

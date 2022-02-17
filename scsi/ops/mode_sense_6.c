
static int mode_sense_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int err,dl,buflen,data_len;

	/* DBD */
	dl = (scp->cmnd[1] & 0x08 ? 0 : 8);
        buflen = min(scp->cmnd[4],sizeof(temp));

	/* MEDIUM TYPE */
	temp[1] = 0;
	/* DEVICE-SPECIFIC PARAMETER */
	temp[2] = 0;
	/* BLOCK DESCRIPTOR LENGTH */
	temp[3] = dl;

	err =  mode_sense(dp, scp, &temp[4], buflen - 4, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* MODE DATA LENGTH */
	temp[0] = data_len;
	return response(scp, &temp, data_len);
}

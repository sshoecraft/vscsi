
//static struct vscsi_op vscsi_pass_op = { "pass-through", vscsi_pass };
	
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)
static int vscsi_queue_lck(struct scsi_cmnd *scp, void (*done)(struct scsi_cmnd *)) {
#else
static int vscsi_queue(struct scsi_cmnd *scp, void (*done)(struct scsi_cmnd *)) {
#endif
	struct vscsi_device *dp;

//	dprintk("scp: %p\n", scp);
	dprintk("--------------------------------- %X -----------------------------------------\n", scp->cmnd[0]);
	dprintk("channel: %d, id: %d, lun: %d\n", scp->device->channel, scp->device->id, scp->device->lun);

	/* Get the device */
	if (!scp->device->hostdata) {
		dp = vscsi_get_device(scp->device->channel, scp->device->id, scp->device->lun);
		scp->device->hostdata = dp;
	} else
		dp = (struct vscsi_device *) scp->device->hostdata;

	if ((dp->rom == 0) && (dp->type != TYPE_PASS)) {
		dprintk("vscsi_queue: NO ROM: cmnd[0]: 0x%02x\n", scp->cmnd[0]);
		if (scp->cmnd[0] == INQUIRY) {
			memset(temp,0,96);
			temp[0] = 0x7f;
			temp[3] = 2;
			temp[4] = 92;
			scp->result = response(scp, temp, min(scp->cmnd[4],96));
		} else
			scp->result = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
		done(scp);
		return 0;
	}

//	dprintk("op: %X\n", (unsigned char) scp->cmnd[0]);
	scp->result = vscsi_call_op(dp, scp);

	dprintk("result: %d\n", scp->result);
	done(scp);
	return GOOD;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)
static DEF_SCSI_QCMD(vscsi_queue)
#endif


/* Local ops */
static int vscsi_local_open(struct vscsi_file *fp, int read_only) {
	dprintk("vscsi_local_open: path: %s\n", fp->path);
	return 1;
}

static int vscsi_local_close(struct vscsi_file *fp) {
	dprintk("vscsi_local_close: closing file...\n");
	return 1;
}

static int vscsi_local_rw(struct vscsi_file *fp, struct scsi_cmnd *scp, unsigned long long lba, int num, int write) {
	return 1;
}

static int vscsi_local_pass(struct vscsi_file *fp, struct scsi_cmnd *scp) {
	return 1;
}

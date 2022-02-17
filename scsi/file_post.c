
#ifndef VSCSI_HAVE_HOST
#define vscsi_host_open vscsi_local_open
#define vscsi_host_close vscsi_local_close
#define vscsi_host_rw vscsi_local_rw
#define vscsi_host_pass vscsi_local_pass
#endif

static struct vscsi_file_ops vscsi_host_fops = {
	vscsi_host_open,
	vscsi_host_close,
	vscsi_host_rw,
	vscsi_host_pass,
};

static struct vscsi_file_ops vscsi_local_fops = {
	vscsi_local_open,
	vscsi_local_close,
	vscsi_local_rw,
	vscsi_local_pass,
};

static struct vscsi_file_ops vscsi_net_fops = {
	vscsi_net_open,
	vscsi_net_close,
	vscsi_net_rw,
	vscsi_net_pass,
};

static struct vscsi_file *vscsi_file_alloc(char *path, int which) {
	struct vscsi_file *fp;

	dprintk("vscsi_file_alloc: path: %s, which: %d\n", path, which);
	fp = kmalloc(sizeof(struct vscsi_file),GFP_KERNEL);
	if (!fp) return 0;

	strncpy(fp->path, path, sizeof(fp->path)-1);
	switch(which) {
	case 2:
		fp->ops = &vscsi_net_fops;
		break;
	case 1:
		fp->ops = &vscsi_host_fops;
		break;
	case 0:
	default:
		fp->ops = &vscsi_local_fops;
		break;
	}

	return fp;
}

static void vscsi_file_free(struct vscsi_file *fp) {
	kfree(fp);
}

/* File ops macros */
#define vscsi_file_open(f,r) f->ops->open(f,r)
#define vscsi_file_close(f) f->ops->close(f)
#define vscsi_file_rw(f,s,b,n,w) f->ops->rw(f,s,b,n,w)
#define vscsi_file_pass(f,s) f->ops->pass(f,s)


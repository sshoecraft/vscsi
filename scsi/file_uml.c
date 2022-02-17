
/* UML Host funcs */
#ifdef __arch_um__
#if BITS_PER_LONG == 64
typedef long long fd_type_t;
#else
typedef long fd_type_t;
#endif
#include <scsi/sg.h>

static int vscsi_host_open(struct vscsi_file *fp, int read_only) {
	struct openflags flags;
	fd_type_t fd;

	/* Open the file */
	dprintk("vscsi_host_open: BITS_PER_LONG: %d\n", BITS_PER_LONG);
	dprintk("vscsi_host_open: path: %s\n", fp->path);
	flags.r = 1;
	if (!read_only) flags.w = 1;
	fd = os_open_file(fp->path, flags, 0);
	dprintk("vscsi_host_open: fd: %lld\n", (long long)fd);
	if (fd < 0) return 1;
	fp->fd = (void *) fd;

	/* Get the size */
	if (os_file_size(fp->path,&fp->size)) {
		dprintk(KERN_WARNING "vscsi_host_open: os_file_size failed!\n");
		os_close_file(fd);
		return 1;
	}
	dprintk("size: %lld\n", fp->size);

	return 0;
}

static int vscsi_host_close(struct vscsi_file *fp) {
	dprintk("vscsi_host_close: fd: %lld\n", (long long)fp->fd);
	os_close_file((fd_type_t)fp->fd);
	fp->size = 0;
	return 0;
}

#if 0
/* Returns 0 if ok else (DID_ERROR << 16). Sets scp->resid . */
//static int fill_from_dev_buffer(fd_type_t fd, struct scsi_cmnd * scp, unsigned char * arr, int arr_len)
static int fill_from_dev_buffer(fd_type_t fd, struct scsi_cmnd * scp)
{
	int k, req_len, act_len, len, active;
	void * kaddr;
	void * kaddr_off;
	struct scatterlist * sgpnt;
	int arr_len = scp->request_bufflen;
	int bytes;

	if (0 == scp->request_bufflen)
		return 0;
	if (NULL == scp->request_buffer)
		return (DID_ERROR << 16);
	if (! ((scp->sc_data_direction == DMA_BIDIRECTIONAL) ||
	      (scp->sc_data_direction == DMA_FROM_DEVICE)))
		return (DID_ERROR << 16);
	if (0 == scp->use_sg) {
		req_len = scp->request_bufflen;
		act_len = (req_len < arr_len) ? req_len : arr_len;
//		memcpy(scp->request_buffer, arr, act_len);
		bytes = os_read_file(fd, scp->request_buffer, act_len);
		dprintk("fill_from_dev_buffer: bytes: %d\n", bytes);
		if (bytes < 0) {
			panic("READ FILE ERROR");
			return 1;
		}
		if (scp->resid)
			scp->resid -= act_len;
		else
			scp->resid = req_len - act_len;
		return 0;
	}
	sgpnt = (struct scatterlist *)scp->request_buffer;
	active = 1;
	for (k = 0, req_len = 0, act_len = 0; k < scp->use_sg; ++k, ++sgpnt) {
		if (active) {
			kaddr = (unsigned char *) kmap_atomic(sgpnt->page, KM_USER0);
			if (NULL == kaddr)
				return (DID_ERROR << 16);
			kaddr_off = (unsigned char *)kaddr + sgpnt->offset;
			len = sgpnt->length;
			if ((req_len + len) > arr_len) {
				active = 0;
				len = arr_len - req_len;
			}
//			memcpy(kaddr_off, arr + req_len, len);
			bytes = os_read_file(fd, kaddr_off, len);
			dprintk("fill_from_dev_buffer: bytes: %d\n", bytes);
			if (bytes < 0) {
				panic("READ FILE ERROR");
				return 1;
			}
			kunmap_atomic(kaddr, KM_USER0);
			act_len += len;
		}
		req_len += sgpnt->length;
	}
	if (scp->resid)
		scp->resid -= act_len;
	else
		scp->resid = req_len - act_len;
	return 0;
}

/* Returns number of bytes fetched into 'arr' or -1 if error. */
//static int fetch_to_dev_buffer(fd_type_t fd, struct scsi_cmnd * scp, unsigned char * arr, int max_arr_len, rw_func_t func)
static int fetch_to_dev_buffer(fd_type_t fd, struct scsi_cmnd * scp)
{
	int k, req_len, len, fin;
	void * kaddr;
	void * kaddr_off;
	struct scatterlist * sgpnt;
	int max_arr_len = 512;

	if (0 == scp->request_bufflen)
		return 0;
	if (NULL == scp->request_buffer)
		return -1;
	if (! ((scp->sc_data_direction == DMA_BIDIRECTIONAL) ||
	      (scp->sc_data_direction == DMA_TO_DEVICE)))
		return -1;
	if (0 == scp->use_sg) {
		req_len = scp->request_bufflen;
		len = (req_len < max_arr_len) ? req_len : max_arr_len;
//		memcpy(arr, scp->request_buffer, len);
		os_write_file(fd, scp->request_buffer, len);
		return len;
	}
	sgpnt = (struct scatterlist *)scp->request_buffer;
	for (k = 0, req_len = 0, fin = 0; k < scp->use_sg; ++k, ++sgpnt) {
		kaddr = (unsigned char *)kmap_atomic(sgpnt->page, KM_USER0);
		if (NULL == kaddr)
			return -1;
		kaddr_off = (unsigned char *)kaddr + sgpnt->offset;
		len = sgpnt->length;
		if ((req_len + len) > max_arr_len) {
			len = max_arr_len - req_len;
			fin = 1;
		}
//		memcpy(arr + req_len, kaddr_off, len);
		os_write_file(fd, kaddr_off, len);
		kunmap_atomic(kaddr, KM_USER0);
		if (fin)
			return req_len + len;
		req_len += sgpnt->length;
	}
	return req_len;
}
#endif

typedef int (*rw_func_t)(int, void *, int);

static int vscsi_host_rw(struct vscsi_file *fp, struct scsi_cmnd *scp, unsigned long long lba, int num, int write) {
	struct scatterlist *sg;
	int fd = (fd_type_t) fp->fd;
	rw_func_t func;
	unsigned long long offset;
	int i,len;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	struct scsi_data_buffer *sdb = scsi_in(scp);
#else
	char *buffer;
	int buflen;
#endif

	dprintk("vscsi_host_rw: lba: %lld, num: %d, write: %d\n", lba, num, write);

//	if (write) return 1;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	if (!sdb->length) return 0;
	if (!sdb->table.sgl) {
		dprintk("returning DID_ERROR\n");
		return (DID_ERROR << 16);
	}
//	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE)) return (DID_ERROR << 16);
#endif

	offset = lba*scp->device->sector_size;
	len = num*scp->device->sector_size;
	dprintk("vscsi_host_rw: offset: %lld, len: %d\n", offset, len);

	func = (write ? (rw_func_t) os_write_file : (rw_func_t) os_read_file);

	if (os_seek_file(fd,offset) < 0) {
		dprintk(KERN_ERR "vscsi_file_read: file seek error!\n");
		return 1;
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	for_each_sg(sdb->table.sgl, sg, sdb->table.nents, i) {
		if (func(fd,sg_virt(sg),sg->length) < 0) {
			printk(KERN_ERR "vscsi_file_read: file rw error!\n");
			return 1;
		}
	}
#else

#if 0
	if (write) 
		return fetch_to_dev_buffer(fd,scp);
	else
		return fill_from_dev_buffer(fd,scp);
#endif

	/* Scatter/Gather */
	printf("use_sg: %d\n", scp->use_sg);
	if (scp->use_sg) {
		sg = (struct scatterlist *)scp->request_buffer;

		for(i=0; i < scp->use_sg; i++, sg++) {
			buffer = page_address(sg->page) + sg->offset;
			buflen = min(len, sg->length);
			if (func(fd,buffer,buflen) < 0) {
				printk(KERN_ERR "vscsi_file_read: file rw error!\n");
				return 1;
			}
		}
	/* Direct */
	} else {
		buffer = scp->request_buffer;
		buflen = scp->request_bufflen;
		if (func(fd,buffer,buflen) < 0) {
			printk(KERN_ERR "vscsi_file_read: file rw error!\n");
			return 1;
		}
        }
#endif

	return 0;
}

static int vscsi_host_pass(struct vscsi_file *fp, struct scsi_cmnd *scp) {
#if 0
        struct scatterlist *sg;
        unsigned char *buffer = 0;
        int buflen = 0,dir,result;
	int fd = (fd_type_t) fp->fd;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
        struct scsi_data_buffer *sdb = scsi_in(scp);
        int i;

//	printk("sdb->table.sgl: %p\n", sdb->table.sgl);
//	if (!sdb->table.sgl) return (DID_ERROR << 16);
	printk("scsi_bidi_cmnd(scp): %d\n", scsi_bidi_cmnd(scp));
//	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE)) return (DID_ERROR << 16);

	if (sdb->table.sgl) {
		for_each_sg(sdb->table.sgl, sg, sdb->table.nents, i) {
			buffer = sg_virt(sg);
			buflen = sg->length;
			break;
		}
	}

#else
        /* Scatter/Gather */
	printf("use_sg: %d\n", scp->use_sg);
        if (scp->use_sg) {
                sg = (struct scatterlist *)scp->request_buffer;
                buffer = page_address(sg->page) + sg->offset;
                buflen = sg->length;

        /* Direct */
        } else {
                buffer = scp->request_buffer;
                buflen = scp->request_bufflen;
        }
#endif
//        if (!buffer) return (DID_ERROR << 16);
//        if (!buflen) return GOOD;

	if (scp->sc_data_direction == DMA_FROM_DEVICE)
		dir = -1;
	else if (scp->sc_data_direction == DMA_NONE)
		dir = 0;
	else
		dir = 1;
	printk("dir: %d\n", dir);
	result = os_sgio(fd, scp->cmnd, scp->cmd_len, buffer, buflen, dir);
	if (result == 0 && dir < 0) dump_data(0, "pass response", buffer, buflen);
	return result;
#else
	return 1;
#endif
}

#define VSCSI_HAVE_HOST 1
#endif /* !__arch_um__ */

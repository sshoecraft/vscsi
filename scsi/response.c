
static int xfer_to_dev(struct scsi_cmnd *scp, void *data, int data_len) {
	struct scatterlist *sg;
	unsigned char *buffer = 0;
	int buflen = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	struct scsi_data_buffer *sdb = scsi_in(scp);
	int i;

	if (!sdb->length) return 0;
	if (!sdb->table.sgl) return (DID_ERROR << 16);
	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_TO_DEVICE)) return (DID_ERROR << 16);

	for_each_sg(sdb->table.sgl, sg, sdb->table.nents, i) {
		buffer = sg_virt(sg);
		buflen = sg->length;
		break;
	}
#else
	/* Scatter/Gather */
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
	if (!buffer) return (DID_ERROR << 16);
	if (!buflen) return GOOD;

	dump_data(scp->device->id, "xfer_to_dev", data, min(buflen,data_len));
	memcpy(data, buffer, min(buflen, data_len));

	return GOOD;
}

static int response(struct scsi_cmnd *scp, void *data, int data_len) {
	struct scatterlist *sg;
	unsigned char *buffer = 0;
	int buflen = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	struct scsi_data_buffer *sdb = scsi_in(scp);
	int i;

	if (!sdb->length) return 0;
	if (!sdb->table.sgl) return (DID_ERROR << 16);
	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE)) return (DID_ERROR << 16);

	for_each_sg(sdb->table.sgl, sg, sdb->table.nents, i) {
		buffer = sg_virt(sg);
		buflen = sg->length;
		break;
	}
#else
	/* Scatter/Gather */
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
	if (!buffer) return (DID_ERROR << 16);
	if (!buflen) return GOOD;

	dprintk("buflen: %d, data_len: %d\n", buflen, data_len);
	dump_data(scp->device->id, "reponse", data, min(buflen,data_len));
	memcpy(buffer, data, min(buflen, data_len));

	return GOOD;
}

#if 0
		if (active) {
			kaddr = (unsigned char *) kmap_atomic(sg_page(sg), KM_USER0);
			if (!kaddr) return (DID_ERROR << 16);
			kaddr_off = (unsigned char *)kaddr + sg->offset;
			len = sg->length;
			if ((req_len + len) > data_len) {
				active = 0;
				len = data_len - req_len;
			}
			memcpy(kaddr_off, data + req_len, len);
			kunmap_atomic(kaddr, KM_USER0);
			act_len += len;
		}
		req_len += sg->length;
#endif

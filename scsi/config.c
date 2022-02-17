
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
static int vscsi_config(struct scsi_device *device) {

	dprintk("vscsi_config: id: %d\n", device->id);

//	sdev->use_10_for_rw = 1;
	device->use_10_for_ms = 1;
//	scsi_adjust_queue_depth(sdev, MSG_SIMPLE_TAG, sdev->host->cmd_per_lun);
#if 0
	switch(sdev->type) {
	case TYPE_ROM:
	case TYPE_WORM:
		/* XXX required to get rid of "unaligned transfer" errors */
	        blk_queue_hardsect_size(sdev->request_queue, 2048);
		break;
	case TYPE_DISK:
	default:
		break;
	}
#endif

#ifdef DO_SPI
	if (spi_support_sync(device->sdev_target) && !spi_initial_dv(device->sdev_target)) {
		spi_dv_device(device);
		spi_display_xfer_agreement(device->sdev_target);
	}
#endif
	return 0;
}
#endif

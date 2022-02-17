
/* Keep sg size to <= 1 page */
#define VSCSI_SGSIZE ( 4096 / sizeof(struct scatterlist) )
#define VSCSI_CLUSTERING	1
#define VSCSI_DUMP_PARAMS	1

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
struct scsi_host_template vscsi_template = {
	.module			= THIS_MODULE,
	.name			= "Virtual SCSI Adapter",
	.proc_name		= "vscsi",
	.queuecommand		= vscsi_queue,
	.slave_configure	= vscsi_config,
	.skip_settle_delay	= 1,
	.this_id		= -1,
	.sg_tablesize		= VSCSI_SGSIZE,
	.max_sectors		= 0xFFFF,
	.can_queue		= 65535,
	.cmd_per_lun		= 2048,
	.use_clustering		= VSCSI_CLUSTERING,
	.eh_device_reset_handler = vscsi_device_reset,
};
#else
/* XXX 2.4 template */
static void setup_host(struct Scsi_Host *);
static int vscsi_detect(Scsi_Host_Template *shtp) { setup_host(scsi_register(shtp, 0)); return 1; }
static int vscsi_release(struct Scsi_Host *shp) { scsi_unregister(shp); return 0; }
static Scsi_Host_Template vscsi_template = {
	.module			= THIS_MODULE,
	.proc_name		= "vscsi",
//	proc_info:	scsi_debug_proc_info,
	.name			= "Virtual SCSI Adapter",
	.detect			= vscsi_detect,
	.release		= vscsi_release,
//	info:              scsi_debug_info,
//	ioctl:             scsi_debug_ioctl,
	.queuecommand		= vscsi_queue,
//	eh_abort_handler:  scsi_debug_abort,
//	eh_bus_reset_handler: scsi_debug_bus_reset,
	.eh_device_reset_handler = vscsi_device_reset,
//	eh_host_reset_handler: scsi_debug_host_reset,
//	bios_param:        scsi_debug_biosparam,
	can_queue:         255,
	this_id:           7,
	sg_tablesize:      64,
	cmd_per_lun:       3,
	unchecked_isa_dma: 0,
	use_clustering:    VSCSI_CLUSTERING,
	use_new_eh_code:   1,
};
#endif

static void setup_host(struct Scsi_Host *shost) {
	/* Set params */
	shost->max_channel = VSCSI_NUM_CHANS-1;
	shost->max_id = VSCSI_NUM_IDS;
	shost->max_lun = VSCSI_NUM_LUNS;

	shost->io_port = 0;
	shost->n_io_port = 0;
	shost->dma_channel = -1;

#ifdef DO_SPI
	shost->transportt = vscsi_transport_template;
#endif

#if VSCSI_DUMP_PARAMS
#define SDUMP(s,f) dprintk(KERN_INFO "  %16s: %d\n", #f, (s)->f)
	dprintk(KERN_INFO "host parameters:\n");
	SDUMP(shost,max_id);
	SDUMP(shost,max_lun);
	SDUMP(shost,max_channel);
	SDUMP(shost,unique_id);
	SDUMP(&vscsi_template,can_queue);
	SDUMP(&vscsi_template,cmd_per_lun);
	SDUMP(&vscsi_template,sg_tablesize);
	SDUMP(&vscsi_template,max_sectors);
	SDUMP(&vscsi_template,use_clustering);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	SDUMP(shost,use_blk_tcq);
	SDUMP(&vscsi_template,max_host_blocked);
#endif
#undef SDUMP
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int init_adapter(struct device *dev) {
	struct Scsi_Host *shost;
	int rc;

	/* Get shost */
	dprintk("vSCSI: allocating host...\n");
	shost = scsi_host_alloc(&vscsi_template, sizeof(void *));
	if (!shost) {
		dprintk(KERN_ERR "vSCSI: init_adapter: scsi_host_alloc failed\n");
		return -ENOMEM;
	}
//	dprintk("vSCSI: shost: %p\n", shost);

	/* Setup host parms */
	setup_host(shost);

	/* Add host */
	dprintk("vSCSI: adding host...\n");
	rc = scsi_add_host(shost, dev);
	if (rc) {
		dprintk(KERN_ERR "vSCSI: init_adapter: scsi_add_host failed\n");
		goto err_put;
	}
	dev_set_drvdata(dev, shost);

	/* Scan devs */
	dprintk("vSCSI: Scanning...\n");
	scsi_scan_host(shost);

//	exit(1);
	return 0;

err_put:
	scsi_host_put(shost);
	return rc;
}
#endif

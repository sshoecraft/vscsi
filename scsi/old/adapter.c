
/* Keep sg size to <= 1 page */
#define VSCSI_SGSIZE ( 4096 / sizeof(struct scatterlist) )
#define VSCSI_CLUSTERING	0
#define VSCSI_DUMP_PARAMS	1

struct scsi_host_template vscsi_template = {
	.module			= THIS_MODULE,
	.name			= "Virtual SCSI Adapter",
	.proc_name		= "vscsi",
	.queuecommand		= vscsi_queue,
	.slave_configure	= vscsi_config,
	.skip_settle_delay	= 1,
	.this_id		= -1,
	.sg_tablesize		= (VSCSI_CLUSTERING ? 0 : VSCSI_SGSIZE),
	.max_sectors		= 0xFFFF,
	.can_queue		= 65535,
	.cmd_per_lun		= 2048,
	.use_clustering		= VSCSI_CLUSTERING,
	.eh_device_reset_handler = vscsi_device_reset,
};

static void setup_host(struct Scsi_Host *shost) {
	/* Set params */
	shost->max_channel = VSCSI_NUM_CHANS-1;
	shost->max_id = VSCSI_NUM_IDS;
	shost->max_lun = VSCSI_NUM_LUNS;

	shost->io_port = 0;
	shost->n_io_port = 0;
	shost->dma_channel = -1;

	/* MUST be set for *_16 ops */
	shost->max_cmd_len = 16;

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

//static int init_adapter(struct platform_device *dev) {
static struct Scsi_Host *init_adapter(struct device *dev) {
	struct Scsi_Host *shost;

	/* Get shost */
	dprintk("vSCSI: allocating host...\n");
	shost = scsi_host_alloc(&vscsi_template, sizeof(void *));
	if (!shost) {
		dprintk(KERN_ERR "vSCSI: init_adapter: scsi_host_alloc failed\n");
		return 0;
	}
//	dprintk("vSCSI: shost: %p\n", shost);

	/* Setup host parms */
	setup_host(shost);

	/* Add host */
	dprintk("vSCSI: adding host...\n");
	if (scsi_add_host(shost, dev)) {
		dprintk(KERN_ERR "vSCSI: init_adapter: scsi_add_host failed\n");
		goto err_put;
	}
//	platform_set_drvdata(dev, shost);

	/* Scan devs */
	dprintk("vSCSI: Scanning...\n");
	scsi_scan_host(shost);

	return shost;

err_put:
	scsi_host_put(shost);
	return 0;
}

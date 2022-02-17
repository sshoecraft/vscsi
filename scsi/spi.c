#ifdef DO_SPI

static struct scsi_transport_template *vscsi_transport_template;

static void vscsi_set_period(struct scsi_target *target, int period)
{
//	printk("vscsi_set_period: period: %d\n", period);
	return;
}

static void vscsi_set_offset(struct scsi_target *target, int offset)
{
//	printk("vscsi_set_offset: offset: %d\n", offset);
	return;
}

static void vscsi_set_width(struct scsi_target *target, int width)
{
//	printk("vscsi_set_width: width: %d\n", width);
	return;
}

static void vscsi_get_signalling(struct Scsi_Host *shost)
{
	spi_signalling(shost) = SPI_SIGNAL_LVD;
}

static void vscsi_set_dt(struct scsi_target *starget, int dt)
{
//	printk("vscsi_set_dt: dt: %d\n", dt);
#if 0
	struct Scsi_Host *shost = dev_to_shost(starget->dev.parent);
	struct sym_hcb *np = sym_get_hcb(shost);
	struct sym_tcb *tp = &np->target[starget->id];

	/* We must clear QAS and IU if DT is clear */
	if (dt)
		tp->tgoal.dt = 1;
	else
		tp->tgoal.iu = tp->tgoal.dt = tp->tgoal.qas = 0;
	tp->tgoal.check_nego = 1;
#endif
}

static void vscsi_get_dt(struct scsi_target *starget)
{
//	spi_iu(starget) = 1;
//	spi_dt(starget) = 1;
	spi_width(starget) = 1;
//	spi_qas(starget) = 1;
//	spi_wr_flow(starget) = 1;
//	spi_rd_strm(starget) = 1;
	spi_offset(starget) = 2;
	spi_period(starget) = 7;
}

static struct spi_function_template vscsi_transport_functions = {
        .set_offset     = vscsi_set_offset,
        .show_offset    = 1,
        .set_period     = vscsi_set_period,
        .show_period    = 1,
        .set_width      = vscsi_set_width,
        .show_width     = 1,
	.get_dt		= vscsi_get_dt,
        .set_dt         = vscsi_set_dt,
        .show_dt        = 1,
        .get_signalling = vscsi_get_signalling,
};
#endif /* DO_SPI */


#define VSCSI_DEBUG_RW 1

static int read_write(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	printk("read_write: dp: %p, scp: %p\n", dp, scp);

	lba = num = 0;
	switch(*p) {
	case READ_16:
	case WRITE_16:
		{
			register int x;

			for (x = 0; x < 8; x++) {
				if (x) lba <<= 8;
				lba |= *(p+2+x);
			}
			num = *(p+10) << 24 | *(p+11) << 16 | *(p+12) << 8 | *(p+13);
		}
		break;
	case READ_12:
	case WRITE_12:
		lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
		num = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);
		break;
	case READ_10:
	case WRITE_10:
		lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
		num = *(p+7) << 8 | *(p+8);
		break;
	case READ_6:
	case WRITE_6:
		lba = (*(p+1) & 0x1f) << 16 | *(p+2) << 8 | *(p+3);
		num = *(p+4) ? *(p+4) : 0xff;
		break;
	default:
		printk(KERN_ERR "scsi%d: read_write: unknown opcode: %x\n", scp->device->id, *p);
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}

#if VSCSI_DEBUG_RW
	if (DFLAG(dp,DEBUG)) printk(KERN_INFO "read_write: lba: %lld, num: %ld\n", lba, num);
#endif

	if (vscsi_file_rw(dp->fp, scp, lba, num, (scp->cmnd[0] & 2) >> 1))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}


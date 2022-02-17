
#if 0
static unsigned short dvd_features[] = {
	0x00,		/* List */
	0x01,		/* Core */
	0x02, 		/* Morphing */
	0x03,		/* Removable */
	0x04,		/* Write Protect */
	0x10,		/* Random Readable */
#if 0
	0x1D,		/* Multi-Read */
#endif
	0x1E,		/* CD */
	0x1F,		/* DVD */
};

static unsigned short supported_profiles[] = {
	0x08,		/* CD-ROM */
	0x10,		/* DVD-ROM */
};
#endif

static int get_configuration(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rt, sfn, alloc_len;
	unsigned char *p = temp;

#if 0
0 OPERATION CODE (46h)
1 Reserved Reserved RT
2 (MSB) Starting Feature Number
3 (LSB)
4 Reserved
5 Reserved
6 Reserved
7
8
(MSB) Allocation Length
(LSB)
9 Control
#endif
	rt = scp->cmnd[1] & 2;
	sfn = (scp->cmnd[2] << 8) | scp->cmnd[3];
	alloc_len = (scp->cmnd[7] << 8) | scp->cmnd[8];

	dprintk("rt: %d, sfn: %d, alloc_len: %d\n", rt, sfn, alloc_len);

	/* An Allocation Length field of zero indicates that no data shall be transferred. This condition shall not be
considered an error. */
	if (alloc_len == 0) return GOOD;

	/* RT 0=all, 1=all current, 2=single */
	memset(temp,0,min(alloc_len,sizeof(temp)));

	if (!DFLAG_ISSET(dp,OPEN) || dp->type != TYPE_ROM) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	switch(rt) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	}

	temp[7] = 0x10;		/* DVD Profile */
	p += 8;

#if 0
Mandatory Features for DVD-ROM
0000h Profile List A list of all Profiles supported by the device
0001h Core Basic Functionality
0002h Morphing The device changes its operational behavior due to external events
0003h Removable Medium The medium may be removed from the device
0010h Random Readable, PP=1 Read ability for storage devices with random addressing.
001Fh DVD Read The ability to read DVD specific structures
0100h Power Management Initiator and device directed power management
0105h Timeout Ability to respond to all commands within a specific time
0107h Real-Time Streaming Ability to read using Initiator requested performance parameters
#endif
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


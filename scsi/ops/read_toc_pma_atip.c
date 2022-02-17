static int read_toc_pma_atip(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int time,format,track,alloc_len;
	unsigned char *p;

#if 0
0 OPERATION CODE (43h)
1 Reserved Reserved TIME Reserved
2 Reserved Format
3 Reserved
4 Reserved
5 Reserved
6 Track/Session Number
7 (MSB) Allocation Length
8 (LSB)
9 Control
#endif

	time = ((scp->cmnd[0] & 2) != 0);
	format = scp->cmnd[2] & 0x0f;
	track = scp->cmnd[6];
	alloc_len = (scp->cmnd[7] << 8) | scp->cmnd[8];
	p = &temp[2];

	dprintk("time: %d, format: %x, track: %d, alloc_len: %d\n", time, format, track, alloc_len);

	switch(format) {
	case 0:
		/* The Track/Session Number field specifies starting track number for which the data is returned. For multi-session discs, this command returns the TOC data for all sessions and for Track number AAh only the Lead-out area of the last complete session. */
#if 0
0 (MSB) TOC Data Length
1 (LSB)
2 First Track Number(Hex)
3 Last Track Number(Hex)
TOC Track Descriptor(s)
0 Reserved
1 ADR CONTROL
2 Track Number(Hex)
3 Reserved
4 (MSB)
… Track Start Address
7 (LSB)
#endif
		/* Header */
		*p++ = 1;		/* First track # */
		*p++ = 1;		/* Last track # */

		/* Track Descriptor for track 0 */
		*p++ = 0;		/* Reserved */
		*p++ = 6;		/* ADR & Control (Data,uninterupted+copy permitted */
		*p++ = 1;		/* Track # */
		*p++ = 0;		/* Reserved */
		*p++ = 0;		/* Track start MSB */
		*p++ = 0;
		*p++ = 0;
		*p++ = 16;		/* Track start LSB */
		temp[1] = p - temp;
		break;
	case 1:
		/* This format returns the first complete session number, last complete session number and last complete session starting address. In this format, the Track/Session Number field is reserved and should be set to 00h. NOTE: This format provides the Initiator access to the last finalized session starting address quickly. */
		break;
	case 2:
		/* This format returns all Q sub-code data in the Lead-In (TOC) areas starting from a session number as specified in the Track/Session Number field. In this mode, the Logical Unit shall support Q Sub-channel POINT field value of A0h, A1h, A2h, Track numbers, B0h, B1h, B2h, B3h, B4h, C0h, and C1h. See Table 236. There is no defined LBA addressing and TIME bit shall be set to one. */
	case 3:
		/* This format returns all Q sub-code data in the PMA area. In this format, the Track/Session Number field is reserved and shall be set to 00h. See Table 241. There is no defined LBA addressing and TIME bit shall be set to one. */
	case 4:
		/* This format returns ATIP data. In this format, the Track/Session Number field is reserved and shall be set to 00h. See Table 242. There is no defined LBA addressing and TIME bit shall be set to one */
	case 5:
		/* This format returns CD-TEXT information that is recorded in the Lead-in area as R-W Sub-channel Data */
	default:
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}

	return response(scp, temp, min(alloc_len, temp[1]));
}


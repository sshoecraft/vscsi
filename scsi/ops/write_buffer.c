
static unsigned char echobuf[4096];
static int echobuf_len = 0;

static int write_buffer(struct vscsi_device *dp, struct scsi_cmnd *scp) {
//	int mode, id, off, len;
	int mode,len;
#if 0
0 OPERATION CODE (3Bh)
1 Reserved MODE
2 BUFFER ID
3 (MSB) BUFFER OFFSET
5 (LSB)
6 (MSB) PARAMETER LIST LENGTH
8 (LSB)
9 CONTROL
#endif
	mode = scp->cmnd[1] & 0xF;
//	id = scp->cmnd[2];
//	off = scp->cmnd[3] << 16 | scp->cmnd[4] << 8 | scp->cmnd[5];
	len = scp->cmnd[6] << 16 | scp->cmnd[7] << 8 | scp->cmnd[8];
//	dprintk("write_buffer: mode: %x, id: %d, off: %x, len: %d\n", mode, id, off, len);

#if 0
6.39.9 Write data to echo buffer mode (0Ah)
In this mode the device server transfers data from the application client and stores it in an echo buffer. An echo
buffer is assigned in the same manner by the device server as it would for a write operation. Data shall be sent
aligned on four-byte boundaries.
The BUFFER ID and BUFFER OFFSET fields shall be ignored in this mode.
NOTE 42 - It is recommended that the logical unit assign echo buffers on a per I_T nexus basis to limit the number
of exception conditions that may occur when I_T nexuses are present.
Upon successful completion of a WRITE BUFFER command the data shall be preserved in the echo buffer unless
there is an intervening command to any logical unit in which case the data may be changed.
The PARAMETER LIST LENGTH field specifies the maximum number of bytes that shall be transferred from the
Data-Out Buffer to be stored in the echo buffer. The application client should ensure that the parameter list length
does not exceed the capacity of the echo buffer. The capacity of the echo buffer is indicated by the BUFFER
CAPACITY field in the READ BUFFER echo buffer descriptor (see 6.16.7). If the PARAMETER LIST LENGTH field
specifies a transfer in excess of the buffer capacity, the command shall be terminated with CHECK CONDITION
status, with the sense key set to ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN
CDB.
#endif

	/* We only support the echo buffer */
	dprintk("write_buffer: mode: %x, len: %d\n", mode, len);
	if (mode == 0x0A) {
		dprintk("write_buffer: len: %d, echobuf: %d\n", len, (int)sizeof(echobuf));
		if (len > sizeof(echobuf))
			return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
		echobuf_len = len;
		return xfer_to_dev(scp, echobuf, sizeof(echobuf));
	}

	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

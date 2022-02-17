static int read_buffer(struct vscsi_device *dp, struct scsi_cmnd *scp) {
//	int mode, id, off, len;
	int mode, len;

#if 0
0 OPERATION CODE (3Ch)
1 Reserved MODE
2 BUFFER ID
3 (MSB) BUFFER OFFSET
5 (LSB)
6 (MSB) ALLOCATION LENGTH
8 (LSB)
9 CONTROL
#endif
	mode = scp->cmnd[1] & 0x0F;
//	id = scp->cmnd[2];
//	off = scp->cmnd[3] << 16 | scp->cmnd[4] << 8 | scp->cmnd[5];
	len = scp->cmnd[6] << 16 | scp->cmnd[7] << 8 | scp->cmnd[8];
//	dprintk("read_buffer: mode: %x, id: %d, off: %d, len: %d\n", mode, id, off, len);

#if 0
6.16.6 Read data from echo buffer mode (0Ah)
In this mode the device server transfers data to the application client from the echo buffer that was written by the
most recent WRITE BUFFER command wit the MODE field set to write data to echo buffer (see 6.39.9) received on
the same I_T nexus. The READ BUFFER command shall return the same number of bytes of data as received in
the prior WRITE BUFFER command with the mode field set to write data to echo buffer, limited by the allocation
length as described in 4.3.5.6.
The BUFFER ID and BUFFER OFFSET fields are ignored in this mode.

If no WRITE BUFFER command with the mode set to write data to echo buffer received on this I_T nexus has
completed without an error, then the READ BUFFER command shall terminate with CHECK CONDITION status,
with the sense key set to ILLEGAL REQUEST, and the additional sense code set to COMMAND SEQUENCE
ERROR. If the data in the echo buffer has been overwritten by another I_T nexus, the READ BUFFER command
shall be terminated with CHECK CONDITION status, with the sense key set to ABORTED COMMAND, and the
additional sense code set to ECHO BUFFER OVERWRITTEN.
After a WRITE BUFFER command with the mode set to write data to echo buffer has completed without an error,
the application client may send multiple READ BUFFER commands with the mode set to read data from echo
buffer in order to read the echo buffer data multiple times.

6.16.7 Echo buffer descriptor mode (0Bh)
In this mode, a maximum of four bytes of READ BUFFER descriptor information is returned. The device server
shall return the descriptor information for the echo buffer. If there is no echo buffer implemented, the device server
shall return all zeros in the READ BUFFER descriptor. The BUFFER ID field and BUFFER OFFSET field are reserved in
this mode. The allocation length should be set to four or greater. The READ BUFFER descriptor is defined as
shown in table 186.

0 Reserved EBOS
1 Reserved
2 Reserved (MSB)
3 BUFFER CAPACITY (LSB)

The BUFFER CAPACITY field shall return the size of the echo buffer in bytes aligned to a four-byte boundary. The
maximum echo buffer size is 4 096 bytes.
If the echo buffer is implemented, the echo buffer descriptor shall be implemented.
An echo buffer overwritten supported (EBOS) bit set to one indicates either:
a) The device server returns the ECHO BUFFER OVERWRITTEN additional sense code if the data being
read from the echo buffer is not the data previously written by the same I_T nexus, or
b) The device server ensures echo buffer data returned to each I_T nexus is the same as that previously
written by that I_T nexus.
An EBOS bit set to zero specifies that the echo buffer may be overwritten by any intervening command received on
any I_T nexus.
A READ BUFFER command with the mode set to echo buffer descriptor may be used to determine the echo buffer
capacity and supported features before a WRITE BUFFER command with the mode set to write data to echo buffer
(see 6.39.9) is sent.
#endif

	/* Echo buffer descriptor mode */
	dprintk("read_buffer: mode: %x, len: %d\n", mode, len);
	if (mode == 0x0A) {
		return response(scp, &echobuf, echobuf_len);
	} else if (mode == 0x0B) {
		int n = sizeof(echobuf);
		temp[0] = 0;
		temp[1] = 0;
		temp[2] = n >> 8;
		temp[4] = n & 0xFF;
		return response(scp, &temp, 4);
	}

	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


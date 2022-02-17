static int start_stop_unit(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int start;

#if 0
0 OPERATION CODE (1Bh)
1 Reserved IMMED
2 Reserved
3 Reserved POWER CONDITION MODIFIER
4 POWER CONDITION Reserved NO_FLUSH LOEJ START
5 CONTROL
#endif
#if 0
POWER CONDITION field
0h START_VALID Process the START and LOEJ bits.
1h ACTIVE Place the device into the active power condition.
2h IDLE Place the device into the idle power condition.
3h STANDBY Place the device into the standby power condition.
4h Reserved Reserved
5h Obsolete Obsolete
6h Reserved Reserved
7h LU_CONTROL Transfer control of power conditions to the logical unit.
8h to 9h Reserved Reserved
Ah FORCE_IDLE_0 Force the idle condition timer to zero.
Bh FORCE_STANDBY_0 Force the standby condition timer to zero.
Ch to Fh Reserved Reserved
#endif
	start = scp->cmnd[4] & 1;
	dprintk("start: %d\n", start);
//	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	return GOOD;
}


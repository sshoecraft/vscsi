/*
Status 		Sense Key 	Additional Sense Code
GOOD 		not applicable 	not applicable
CHECK CONDITION ILLEGAL REQUEST LOGICAL UNIT NOT SUPPORTED
CHECK CONDITION NOT READY 	LOGICAL UNIT DOES NOT RESPOND TO SELECTION
CHECK CONDITION NOT READY 	MEDIUM NOT PRESENT
CHECK CONDITION NOT READY	LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE
CHECK CONDITION NOT READY	LOGICAL UNIT IS IN PROCESS OF BECOMING READY
CHECK CONDITION NOT READY 	LOGICAL UNIT NOT READY, INITIALIZING COMMAND REQUIRED
CHECK CONDITION NOT READY 	LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED
CHECK CONDITION NOT READY	LOGICAL UNIT NOT READY, FORMAT IN PROGRESS

04h/00h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE
04h/01h  DTLPWROMAEBKVF  LOGICAL UNIT IS IN PROCESS OF BECOMING READY
04h/02h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, INITIALIZING COMMAND REQUIRED
04h/03h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED
04h/04h  DTL  RO   B     LOGICAL UNIT NOT READY, FORMAT IN PROGRESS

*/

#define CAUSE_NOT_REPORTABLE 0x00
#define INITIALIZING_COMMAND_REQUIRED 0x02
#define FORMAT_IN_PROGRESS 0x04

static int test_unit_ready(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rc;

	dprintk("test_unit_ready: open: %d\n", DFLAG_ISSET(dp, OPEN));
	if (DFLAG_ISSET(dp, OPEN)) {
		dprintk("test_unit_ready: format: %d\n", DFLAG_ISSET(dp, FORMAT));
		if (DFLAG_ISSET(dp, FORMAT))
			rc = check_condition(dp, NOT_READY, LOGICAL_UNIT_NOT_READY, FORMAT_IN_PROGRESS);
		else
			rc = GOOD;
	} else {
		dprintk("test_unit_ready: removable: %d\n", DFLAG_ISSET(dp, RMB));
		if (DFLAG_ISSET(dp, RMB)) {
//			rc = check_condition(dp, NOT_READY, MEDIUM_NOT_PRESENT, 0);
			rc = GOOD;
		}
		else
			rc = check_condition(dp, NOT_READY, LOGICAL_UNIT_NOT_READY, CAUSE_NOT_REPORTABLE);
	}

	return rc;
}

#if 0
static int unit_ready(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int error, rc;

	rc = GOOD;
	error = (wp->dp->os_handle == 0 ? host_open(wp->dp) : GOOD);
	if (error) {
		switch(wp->dp->type) {
		case TYPE_ROM:
		case TYPE_TAPE:
			rc = check_condition(wp->dp, NOT_READY, MEDIUM_NOT_PRESENT, 0x2);
			break;
		default:
			rc = check_condition(wp->dp, NOT_READY, LOGICAL_UNIT_NOT_READY, 0x2);
			break;
		}
	}

	return rc;
}
#endif


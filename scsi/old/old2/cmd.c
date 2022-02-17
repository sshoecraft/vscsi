
#include "scsi.h"

/* Validate the command */
int validate_command(scsi_device_t *dp, char cmd) {
#if 0
	unsigned char *vp;

	/* Each type has a bitmask specifying which commands are "valid" */
	vp = &valid_commands[dp->type];
	byte = cmd/8;
	bit = cmd % 8;
	printf("cmd: %d, byte: %d, bit: %d\n", cmd, byte, bit);
#endif
}

int scsi_command(char *cdb, int cdb_len) {
	return 0;
}

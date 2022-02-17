
#include "scsi_defs.h"

static char *scsi_device_type_names[SCSI_DEVICE_TYPE_MAX+1] = {
	"disk",				/* 00 */
	"tape",				/* 01 */
	"printer",			/* 02 */
	"processor",			/* 03 */
	"worm",				/* 04 */
	"cd",				/* 05 */
	"optical",			/* 06 */
	"changer",			/* 07 */
	"array",			/* 08 */
	"enclosure",			/* 09 */
	"simple",			/* 10 */
	"reader",			/* 11 */
	"automation",			/* 12 */
	"object",			/* 13 */
};

char *scsi_device_type_name(int type) {
	if (type < 0 || type > SCSI_DEVICE_TYPE_MAX)
		return 0;
	else
		return scsi_device_type_names[type];
}

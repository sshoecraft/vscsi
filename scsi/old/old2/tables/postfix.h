
#ifndef __HAVE_TABLES
extern const char *scsi_device_types[SDT_MAX+1];
#endif /* __HAVE_TABLES */

/* Utility functions */
const char *scsi_device_type(int);
const char *scsi_vendor_name(char *);

#endif /* !__SCSI_TABLES_H */

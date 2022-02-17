
struct scsi_vendor_info {
	char *id;
	char *name;
};

#ifndef __HAVE_TABLES
extern struct scsi_vendor_info scsi_vendors[];
#endif /* __HAVE_TABLES */

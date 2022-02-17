
const char *scsi_device_type(int type) {
	if (type >= 0 && type <= SDT_MAX)
		return(scsi_device_types[type]);
	else
		return("");
}

/* Vendor name quirk table */
static struct scsi_vendor_info scsi_vendor_quirks[] = {
	{ "EXAAYTE","Exabyte Corp." },
	{ 0,0 }
};

const char *scsi_scsi_vendor_name(char *id) {
	register int x;

#ifdef DEBUG_SCSI_VENDOR_NAME
	printf("scsi_scsi_vendor_name: id(%d): %s\n",strlen(id),id);
#endif

	/* Look in the scsi_vendors table */
	for(x=0; scsi_vendors[x].id; x++) {
#ifdef DEBUG_SCSI_VENDOR_NAME
		printf("scsi_vendors[%d].id(%d): %s\n",x,
			strlen(scsi_vendors[x].id),scsi_vendors[x].id);
#endif
		if (strcmp(scsi_vendors[x].id,id)==0)
			return scsi_vendors[x].name;
	}

	/* Not found, try the scsi_vendor quirks table */
	for(x=0; scsi_vendor_quirks[x].id; x++) {
#ifdef DEBUG_SCSI_VENDOR_NAME
		printf("scsi_vendor_quirks[%d].id(%d): %s\n",x,
			strlen(scsi_vendor_quirks[x].id),scsi_vendor_quirks[x].id);
#endif
		if (strcmp(scsi_vendor_quirks[x].id,id)==0)
			return scsi_vendor_quirks[x].name;
	}
	return("");
}

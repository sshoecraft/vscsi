
struct sense_code_info {
	int asc,ascq;
	char *message;
};

#ifndef __HAVE_TABLES
extern struct sense_code_info sense_codes[];
#endif /* __HAVE_TABLES */

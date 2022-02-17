
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct {
	char id[16];
	char name[64];
} vendor;

int main(void) {
	FILE *fp,*fp2;
	char line[128],temp[64];
	register char *ptr,*eptr;
	register int x,state;

	fp = fopen("vendorid.txt","r");
	if (!fp) {
		perror("fopen vendorid.txt");
		return 1;
	}

	/* Create the include file */
	fp2 = fopen("vendors.h","w+");
	if (!fp2) {
		perror("fopen vendors.h for write");
		return 1;
	}
	fprintf(fp2,"\nstruct scsi_vendor_info {\n");
	fprintf(fp2,"\tchar *id;\n");
	fprintf(fp2,"\tchar *name;\n");
	fprintf(fp2,"};\n");
	fprintf(fp2,"\n#ifndef __HAVE_TABLES\n");
	fprintf(fp2,"extern struct scsi_vendor_info scsi_vendors[];\n");
	fprintf(fp2,"#endif /* __HAVE_TABLES */\n");
	fclose(fp2);

	/* Create the source file */
	fp2 = fopen("vendors.c","w+");
	if (!fp2) {
		perror("fopen vendors.c");
		fclose(fp);
		return 1;
	}
	fprintf(fp2,"\nstatic struct scsi_vendor_info scsi_vendors[] = {\n");
	while(fgets(line,sizeof(line),fp)) {
		if (strstr(line,"---------------------------------------"))
			break;
	}
	while(fgets(line,sizeof(line),fp)) {
		ptr = line;
		while(isspace(*ptr)) ptr++;
		x=0;
		while(!isspace(*ptr) && x < sizeof(vendor.id)-1)
			vendor.id[x++] = toupper(*ptr++);
		vendor.id[x] = 0;

		while(isspace(*ptr)) ptr++;
		x=0;
		eptr = line + strlen(line)-1;
		while(isspace(*eptr)) eptr--;
		while(ptr < eptr+1 && x < sizeof(vendor.name)-1) {
			if (*ptr == '(') break;
			vendor.name[x++] = *ptr++;
		}
		ptr = &vendor.name[x-1];
		while(isspace(*ptr)) ptr--;
		*(ptr+1) = 0;

		fprintf(fp2,"\t{ \"%s\",\"%s\" },\n",vendor.id,vendor.name);
	}
//	fprintf(fp2,"\t{ 0,0 }\n");
	fprintf(fp2,"};\n");
	fprintf(fp2,"#define scsi_vendor_count (sizeof(scsi_vendors)/sizeof(struct scsi_vendor_info))\n");
	fclose(fp2);
	fclose(fp);
	return 0;
}

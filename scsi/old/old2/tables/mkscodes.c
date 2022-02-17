
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(void) {
	FILE *fp,*fp2;
	int asc,ascq;
	char line[128],temp[64];
	register char *ptr,*eptr;
	register int x,state;

	fp = fopen("asc-num.txt","r");
	if (!fp) {
		perror("fopen asc-num.txt");
		return 1;
	}

	/* Create the include file */
	fp2 = fopen("scodes.h","w+");
	if (!fp2) {
		perror("fopen scodes.h for write");
		return 1;
	}
	fprintf(fp2,"\nstruct sense_code_info {\n");
	fprintf(fp2,"\tint asc,ascq;\n");
	fprintf(fp2,"\tchar *message;\n");
	fprintf(fp2,"};\n");
	fprintf(fp2,"\n#ifndef __HAVE_TABLES\n");
	fprintf(fp2,"extern struct sense_code_info sense_codes[];\n");
	fprintf(fp2,"#endif /* __HAVE_TABLES */\n");

	/* Create the table */
	fp2 = fopen("scodes.c","w+");
	if (!fp2) {
		perror("fopen scodes.c");
		return 1;
	}
	fprintf(fp2,"\nstruct sense_code_info sense_codes[] = {\n");
	while(fgets(line,sizeof(line),fp)) {
		if (strstr(line,"---------------------------------------"))
			break;
	}
	while(fgets(line,sizeof(line),fp)) {
		temp[0] = '0'; temp[1] = 'x'; temp[4] = 0;
		strncpy(&temp[2],&line[0],2);
		asc = strtol(temp,0,16);
		strncpy(&temp[2],&line[4],2);
		ascq = strtol(temp,0,16);
		if (ascq != 0) continue;
		ptr = line + 25;
		eptr = line + strlen(line);
		for(x=0; x < sizeof(temp); x++) temp[x] = 0;
		x=0;
		while(ptr < eptr && *ptr != '|' && x < sizeof(temp)-1) {
			if (*ptr < 32 || *ptr > 126) {
				ptr++;
				continue;
			}
			temp[x++] = tolower(*ptr++);
		}
		temp[x] = 0;
		if (!strlen(temp)) continue;
#if 0
		temp[0] = toupper(temp[0]);
		ptr = strchr(temp,'/');
		if (ptr) {
			ptr++;
			*ptr = toupper(*ptr);
		}
#endif
		for(ptr = temp; *ptr; ptr++) {
			if (*ptr == ',' || *ptr == '(' || *ptr == '-') {
				*ptr = 0;
				break;
			}
			if (isspace(*ptr) || !isalnum(*ptr)) *ptr = '_';
			*ptr = toupper(*ptr);
		}
		fprintf(fp2,"#define %s 0x%x\n", temp, asc);
#if 0
		fprintf(fp2,"\t{ 0x%02X, 0x%02X, \"%s\" },\n",
			asc,ascq,temp);
#endif
		if (asc == 127) break;
	}
	fclose(fp);
	fprintf(fp2,"\t{ -1, -1, 0 }\n");
	fprintf(fp2,"};\n");
	fclose(fp2);
	return 0;
}

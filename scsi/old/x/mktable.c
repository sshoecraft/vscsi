
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define OUTFILE "table.c"
#define LIST 0
#define FUNCS 1
#define CAPS "M"

#if 0
    D - DIRECT ACCESS DEVICE (SBC-2)                   device column key
    .T - SEQUENTIAL ACCESS DEVICE (SSC-2)             -------------------
    . L - PRINTER DEVICE (SSC)                        M = Mandatory
    .  P - PROCESSOR DEVICE (SPC)                     O = Optional
    .  .W - WRITE ONCE READ MULTIPLE DEVICE (SBC-2)   V = Vendor specific
    .  . R - CD/DVE DEVICE (MMC-3)                    Z = Obsolete
    .  .  O - OPTICAL MEMORY DEVICE (SBC-2)
    .  .  .M - MEDIA CHANGER DEVICE (SMC-2)
    .  .  . A - STORAGE ARRAY DEVICE (SCC-2)
    .  .  . .E - ENCLOSURE SERVICES DEVICE (SES)
    .  .  .  .B - SIMPLIFIED DIRECT-ACCESS DEVICE (RBC)
    .  .  .  . K - OPTICAL CARD READER/WRITER DEVICE (OCRW)
    .  .  .  .  V - AUTOMATION/DRIVE INTERFACE (ADC)
    .  .  .  .  .F - OBJECT-BASED STORAGE (OSD)
#endif

#define NTYPES 0x14
static int type_keys[NTYPES] = {
	'D',				/* 00h - Disk */
	'T',				/* 01h - Tape */
	'L',				/* 02h - Printer */
	'P',				/* 03h - Processor */
	'W',				/* 04h - WORM */
	'R',				/* 05h - CD/DVD */
	'@',				/* 06h - Scanner */
	'O',				/* 07h - Optical */
	'M',				/* 08h - Changer */
	'C',				/* 09h - Comm */
	'@',				/* 0Ah - Obsolete */
	'@',				/* 0Bh - Obsolete */
	'A',				/* 0Ch - Raid */
	'E',				/* 0Dh - Enclosure */
	'B',				/* 0Eh - Simple Disk */
	'K',				/* 0Fh - Card Reader */
	'@',				/* 10h - Bridge */
	'F',				/* 11h - OSD */
	'V',				/* 12h - Automation */
	'@',				/* 13h - Security */
};
unsigned char bm[NTYPES][32];

char *type_names[NTYPES] = {
	"Disk",
	"Tape",
	"Printer",
	"Processor",
	"WORM",
	"CD/DVD",
	"Scanner",
	"Optical",
	"Changer",
	"Communication",
	"Obsolete",
	"Obsolete",
	"Raid",
	"Enclosure",
	"Simple Disk",
	"Card Reader",
	"Bridge",
	"OSD",
	"Automation",
	"Security"
};

int main(void) {
	FILE *fp;
	char line[128], temp[6], *p;
	char op[3],cap[20],desc[60];
	int cap_start,desc_start,*type_index,type;
	int val,byte,bit,mask,found,ncaps,max_count,max_op;
	char *ops[256];
	register int x,y;

	fp = fopen("op-num.txt","r");
	if (!fp) {
		perror("mktable: fopen op-num.txt");
		return 1;
	}
	ncaps = 0;
	while(fgets(line,sizeof(line),fp)) {
		line[strlen(line)-1] = 0;
		if (strncmp(line,"OP ",3)==0) {
			/* Get cap & desc start indexes */
			p = line;
			while(!isspace(*p)) p++;
			while(isspace(*p)) p++;
			cap_start = p - line;
			while(!isspace(*p)) {
				ncaps++;
				p++;
			}
			while(isspace(*p)) p++;
			desc_start = p - line;
			break;
		}
	}
//	printf("ncaps: %d\n", ncaps);
	if (!ncaps) {
		fclose(fp);
		perror("mktable: read op-num header");
		return 1;
	}

	/* Create cap to type index */
	memset(cap, 0, sizeof(cap));
	strncpy(cap, &line[cap_start], ncaps);
//	printf("cap(%d): %s\n", strlen(cap), cap);
	type_index = (int *) malloc(ncaps * sizeof(int));
	if (!type_index) {
		perror("type_index malloc");
		return 1;
	}
	for(x=0; x < NTYPES; x++) type_index[x] = -1;
	for(x=0; x < strlen(cap); x++) {
		for(y=0; y < NTYPES; y++) {
			if (type_keys[y] == cap[x]) {
//				printf("type: %c, loc: %d, key: %c, key_loc: %d\n", cap[x], x, type_keys[y], y );
				type_index[x] = y;
				break;
			}
		}
	}
	for(x=0; x < ncaps; x++) {
		if (type_index[x] == -1) {
			printf("type_index[%d]: %d\n", x, type_index[x]);
			printf("error creating type_index!\n");
			return 1;
		}
	}

	memset(bm,0,sizeof(bm));
	memset(ops,0,sizeof(ops));
	max_count = max_op = 0;
	fgets(line,sizeof(line),fp);
	while(fgets(line,sizeof(line),fp)) {
		if (strlen(line) < 20) continue;
//		printf("line: %s", line);
		memset(op, 0, sizeof(op));
		strncpy(op, line, 2);
		memset(cap, 0, sizeof(cap));
		strncpy(cap, &line[cap_start], ncaps);
		memset(desc, 0, sizeof(desc));
		strcpy(desc, &line[desc_start]);
		if (!strlen(desc)) continue;
		for(x=0; x < strlen(desc); x++) {
			if (!isalnum(desc[x])) desc[x] = '_';
			desc[x] = tolower(desc[x]);
		}
		if (strncmp(desc,"_usage_proposed",10)==0) continue;
		/* Remove trailing underscore */
		while(desc[x-1] == '_') {
			desc[x-1] = 0;
			x--;
		}
		/* Remove __ from anywhere in the string */
		do {
			p = strstr(desc,"__");
			if (p) strcpy(p,p+1);
		} while(p);
//		printf("op: %s, cap: %s, desc: %s\n", op, cap, desc);
		sprintf(temp,"0x%s", op);
		sscanf(temp, "0x%x", &val);
		if (val < 0 || val > 255) {
			printf("invalid val: %d, from line: %s", val, line);
			continue;
		}
		if (val == 127) strcpy(desc,"variable_length_cdb");
		byte = val / 8;
		bit = val % 8;
		mask = 1 << bit;
//		printf("val: %d, byte: %d, bit: %d, mask: %x\n", val, byte, bit, mask);
		found = 0;
		for(x=0; x < strlen(cap); x++) {
//			printf("cap[%d]: %c\n", x, cap[x]);
			if (strchr(CAPS,cap[x])) {
				type = type_index[x];
//				printf("type: %d\n", type);
				bm[type][byte] |= mask;
				found=1;
			}
		}
//		printf("found: %d\n", found);
		if (found && ops[val] == 0) {
			ops[val] = malloc(strlen(desc)+1);
			strcpy(ops[val], desc);
			if (val > max_op) max_op = val;
		}
        }
	fclose(fp);
	max_op++;
#if LIST
	fp = fopen("list","w+");
	if (fp) {
		for(x=0; x < max_op; x++) {
			if (ops[x])
				fprintf(fp,"%s\n", ops[x]);
		}
		fclose(fp);
	}
#endif
#if FUNCS
	for(x=0; x < max_op; x++) {
		if (ops[x]) {
			sprintf(line,"%s.c",ops[x]);
//			printf("Checking for: %s...\n", line);
			fp = fopen(line,"r");
			if (fp) {
				fclose(fp);
				continue;
			}
//			printf("Creating file...\n");
			fp = fopen(line,"w+");
			if (!fp) {
				perror("fopen func");
				exit(1);
			}
			fprintf(fp,"int %s(struct vscsi_device *dp, struct scsi_cmnd *scp) {\n", ops[x]);
			fprintf(fp,"\treturn check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);\n");
			fprintf(fp,"}\n\n");
			fclose(fp);
//			printf("Adding to Makefile...\n");
			fp = fopen("Makefile.srcs","a+");
			if (!fp) {
				perror("fopen Makefile.srcs");
				exit(1);
			}
			fprintf(fp,"SRCS += %s\n", line);
			fclose(fp);
		}
	}
#endif
	fp = fopen(OUTFILE,"w+");
	if (!fp) {
		sprintf(line,"mktable fopen %s", OUTFILE);
		perror(line);
		return 1;
	}
	fprintf(fp,"#define VSCSI_NTYPES %d\n", NTYPES);
	fprintf(fp,"#if VSCSI_STRICT\n");
	fprintf(fp,"static unsigned char scsi_commands[VSCSI_NTYPES][32] = {\n");
	for(x=0; x < NTYPES; x++) {
		fprintf(fp,"\t{ /* %02d - %s */", x, type_names[x]);
		for(y=0; y < 32; y++) {
			if ((y % 8)==0) fprintf(fp,"\n\t\t");
			fprintf(fp,"0x%02X, ", bm[x][y]);
		}
		fprintf(fp,"\n\t},\n");
	}
	fprintf(fp,"};\n");
	fprintf(fp,"#endif\n\n");
	fprintf(fp,"typedef int (*vscsi_opfunc_t)(struct vscsi_device *, struct scsi_cmnd *);\n\n");
	fprintf(fp,"#define VSCSI_MAX_OP 0x%02x\n", max_op-1);
//	fprintf(fp,"static vscsi_op_t vscsi_ops[VSCSI_MAX_OP+1] = {\n");
	fprintf(fp,"struct vscsi_op {\n");
	fprintf(fp,"\tchar *name;\n");
	fprintf(fp,"\tvscsi_opfunc_t func;\n");
	fprintf(fp,"} vscsi_ops[VSCSI_MAX_OP+1] = {\n");
	for(x=0; x < max_op; x++) {
		p = (ops[x] ? ops[x] : "0");
		if (ops[x])
			sprintf(desc,"\"%s\"", ops[x]);
		else
			sprintf(desc,"0");
		fprintf(fp,"\t{ %s,%s },",desc,p);
		y = strlen(p)+1;
		while(y < 60) {
			fprintf(fp,"\t");
			y += 8;
		}
		fprintf(fp,"/* %02x */\n",x);
	}
	fprintf(fp,"};\n");
	fclose(fp);
	return 0;
}

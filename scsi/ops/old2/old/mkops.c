
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "list.h"

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#define DISK 1
#define TAPE 1
#define CD 1
#define CHANGER 1

#if DEBUG
#define DPRINTF(m) printf("%s(%d): ",__FUNCTION__,__LINE__), printf m
#else
#define DPRINTF(fmt,...) /* noop */
#endif

/*
	The purpose of this program is to go through each scsi operation and build a
	function table for each operation and what that operation applies to
	(disk/cd/tape/changer)
*/

struct func_info {
	char *name;			/* Function name */
	int (*func)(void);		/* Function pointer */
	unsigned char opcode;		/* Opcode */
	unsigned char type;		/* Type of device */
};

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

OP  DTLPWROMAEBKVF  Description

#endif

#define DISK_LOC 4
#define TAPE_LOC 5
#define CD_LOC 9
#define CHANGER_LOC 11
#define DESC_LOC 20

#define OP_DISK		0x01
#define OP_CD		0x02
#define OP_TAPE		0x04
#define OP_CHANGER	0x08
#define OP_ALL		(OP_DISK|OP_CD|OP_TAPE|OP_CHANGER)

#define CAPS "MO"

char line[128],temp[64];
char *ops[256];
list func_map,file_map,files;

char *fixdesc(char *);

struct map_info {
	char name[64];
	char newname[64];
};

void disp_map(list l, char *name) {
	struct map_info *mapp;

	printf("%s map:\n",name);
	list_reset(l);
	while((mapp = list_get_next(l)) != 0)
		printf("%s = %s\n", mapp->name, mapp->newname);
}

int read_map(list l, char *name) {
	struct map_info map;
	FILE *fp;
	char *p;

	/* Read the funcs map */
	sprintf(temp,"%s.map",name);
	fp = fopen(temp,"r");
	if (!fp) {
		char msg[128];
		sprintf(msg,"fopen(r) %s",temp);
		perror(msg);
		return 1;
	}
	while(fgets(line,sizeof(line),fp)) {
		line[strlen(line)-1] = 0;
		p = strchr(line,' ');
		if (!p) continue;
		*p++ = 0;
		strcpy(map.name,line);
		strcpy(map.newname,p);
		list_add(l,&map,sizeof(map));
	}
	fclose(fp);
//	disp_map(l,name);
	return 0;
}

void addop(int num, char *name) {
	struct map_info *mapp;
	int found;
	char *p;

//	printf("addop: num: %d, name: %s\n", num, name);
	found = 0;
	list_reset(func_map);
	while((mapp = list_get_next(func_map)) != 0) {
//		printf("addop: mapp->name: %s\n", mapp->name);
		if (strcmp(mapp->name,name) == 0) {
//			printf("addop: new name: %s\n", mapp->newname);
			found=1;
			break;
		}
	}
	p = (found ? mapp->newname : name);
//	printf("addop: p: %s\n", p);
	ops[num] = malloc(strlen(p)+1);
	strcpy(ops[num],p);
}

int addfile(char *name) {
	struct map_info *mapp;
	int found;
	char *p,*p2;

//	printf("addfile: name: %s\n", name);
	found = 0;
	list_reset(file_map);
	while((mapp = list_get_next(file_map)) != 0) {
//		printf("addfile: mapp->name: %s\n", mapp->name);
		if (strcmp(mapp->name,name) == 0) {
//			printf("addfile: newname: %s\n", mapp->newname);
			found=1;
			break;
		}
	}
	p = (found ? mapp->newname : name);
//	printf("addfile: p: %s\n", p);

	found = 0;
	list_reset(files);
	while((p2 = list_get_next(files)) != 0) {
		if (strcmp(p,p2) == 0) {
			found=1;
			break;
		}
	}
	if (!found) {
//		printf("addfile: adding: %s\n", p);
		list_add(files,p,strlen(p)+1);
	}
	return 0;
}

int main(int argc, char **argv) {
	FILE *fp,*fp2;
	char cap,*p;
	int op,i,want;
	int caps[32],cap_count;

	func_map = list_create();
	if (read_map(func_map,"func")) return 1;
//	printf("func_map count: %d\n", list_count(func_map));
	file_map = list_create();
	if (read_map(file_map,"file")) return 1;
//	printf("file_map count: %d\n", list_count(file_map));
	files = list_create();

	/* Build a list of caps we want */
	cap_count = 0;
#if DISK
	caps[cap_count++] = DISK_LOC;
#endif
#if TAPE
	caps[cap_count++] = TAPE_LOC;
#endif
#if CD
	caps[cap_count++] = CD_LOC;
#endif
#if CHANGER
	caps[cap_count++] = CHANGER_LOC;
#endif

	fp = fopen("op-num.txt","r");
	if (!fp) {
		perror("fopen(r) op-num.txt");
		return 1;
	}

	/* Skip past header */
	while(fgets(line,sizeof(line),fp)) {
		if (strncmp(line,"--",2) == 0)
			break;
	}
	if (strncmp(line,"--",2) != 0) {
		printf("mktable: unable to find header!\n");
		fclose(fp);
		return 1;
	}

	memset(ops,0,sizeof(ops));
	while(fgets(line,sizeof(line),fp)) {
		line[strlen(line)-1] = 0;
//		printf("line: %s\n", line);
		line[2] = 0;
		sprintf(temp,"0x%s", line);
//		printf("temp: %s\n", temp);
		op = strtol(temp,NULL,16);
//		printf("op: %d\n", op);

		/* Do we want this one? */
		want=0;
		for(i=0; i < cap_count; i++) {
			printf("index: %d\n", caps[i]);
			cap = line[caps[i]];
			if (!cap) cap = 'N';
			printf("CAPS: %s, cap: %c\n", CAPS, cap);
			if (strchr(CAPS,cap)) {
				want=1;
				break;
			}
		}
		printf("want: %d\n", want);
		if (!want) continue;

		if (ops[op]) continue;
		strcpy(temp,&line[DESC_LOC]);
		if (op == 127)
			strcpy(temp,"variable_length_cdb");
		else
			fixdesc(temp);
//		printf("op: %02x, cap: %c, desc: %s\n", op, cap, temp);
		addop(op, temp);
		memset(line,0,sizeof(line));
	}
	fclose(fp);

	/* Add additional ops? */
	fp = fopen("add.txt","r");
	if (fp) {
		while(fgets(line,sizeof(line),fp)) {
			line[strlen(line)-1] = 0;
			line[2] = 0;
			sprintf(temp,"0x%s", line);
			op = strtol(temp,NULL,16);
			printf("*** ADDING: op: %d, func: %s\n", op, &line[3]);
			addop(op,&line[3]);
		}
		fclose(fp);
	}

	/* Build file list */
	for(op=0; op < 256; op++) {
		if (!ops[op])  continue;
		if (addfile(ops[op])) return 1;
	}

	/* Create ops.c */
	fp = fopen("ops.c","w+");
	if (!fp) {
		perror("fopen(w+) ops.c");
		return 1;
	}
	/* Include each op source */
	list_reset(files);
	while((p = list_get_next(files)) != 0) {
		sprintf(temp,"%s.c",p);
//		printf("temp: %s\n", temp);
#if 1
		fp2 = fopen(temp,"r");
		if (!fp2) {
			char temp2[128],cmd[1024];

			sprintf(temp2,"../old/%s.c",p);
			fp2 = fopen(temp2,"r");
			if (!fp2) {
				fp2 = fopen(temp,"w+");
				if (!fp2) perror(temp);
				fprintf(fp2,"static int %s(struct vscsi_device *dp, struct scsi_cmnd *scp) {\n",p);
				fprintf(fp2,"\treturn check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);\n");
				fprintf(fp2,"}\n\n");
				fclose(fp2);
			} else {
				fclose(fp2);
				sprintf(cmd,"mv %s .",temp2);
				system(cmd);
			}
		} else
			fclose(fp2);
#endif

		fp2 = fopen(temp,"r");
		if (!fp2) {
			char msg[1024];
			sprintf(msg,"fopen(r) %s",temp);
			perror(msg);
			return 1;
		}
		while(fgets(line,sizeof(line),fp2)) 
			fprintf(fp,"%s",line);
		fclose(fp2);
	}

	/* Build op table */
	fprintf(fp,"\ntypedef int (*vscsi_opfunc_t)(struct vscsi_device *, struct scsi_cmnd *);\n");
	fprintf(fp,"struct vscsi_op {\n");
	fprintf(fp,"\tchar *name;\n");
	fprintf(fp,"\tvscsi_opfunc_t func;\n");
	fprintf(fp,"} vscsi_ops[256] = {\n");
	for(op=0; op < 256; op++) {
		if (ops[op]) 
			fprintf(fp,"\t/* %02x */ { \"%s\",%s },\n", op, ops[op], ops[op]);
		else
			fprintf(fp,"\t/* %02x */ { 0, 0 },\n", op);
	}
	fprintf(fp,"};\n");
	fclose(fp);

	return 0;
}

char *fixdesc(char *desc) {
	register char *ptr;

	DPRINTF(("**************************\n"));
	DPRINTF(("desc: %s\n", desc));

	/* Replace all non-alnum chars with underscore & lower it */
	for(ptr = desc; *ptr; ptr++) {
		if (isalnum((int)*ptr) == 0 && *ptr != '.')
			*ptr = '_';
		else
			*ptr = tolower(*ptr);
	}
	DPRINTF(("desc: %s\n", desc));

	/* Remove underscores from start */
	if (*desc == '_') {
		for(ptr = desc; *ptr == '_'; ptr++);
		strcpy(desc,ptr);
	}

	/* Remove underscores from end */
	ptr = desc + strlen(desc);
	while(ptr > desc && *(ptr-1) == '_') ptr--;
	*ptr = 0;

	/* Replace all duplicate underscores with a single */
	for(ptr=desc; *ptr; ptr++) {
		if (*ptr == '_' && *(ptr+1) == '_') {
			char *p2;

			for(p2=ptr+1; *p2 == '_'; p2++);
			strcpy(ptr+1,p2);
		}
	}

	DPRINTF(("desc: %s\n",desc));
	return desc;
}

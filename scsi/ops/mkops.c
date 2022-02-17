
#include <stdio.h>
#include <string.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "list.h"

/* Set this to enable simple op debugging */
#define DEBUG_OPS 0

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0

#if DEBUG
#define dprintf(format, args...) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args)
#else
#define dprintf(fmt,...) /* noop */
#endif

char line[128],temp[128];
unsigned char types_start, types_end, desc_start;
unsigned char type_locs[16];
list allops,func_map,file_map,dep_map,files;

char *fixdesc(char *);

struct map_info {
	char name[64];
	char newname[64];
};

struct op_info {
	unsigned char op;
	unsigned short types;
	char name[64];
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

int addfile(char *name) {
	struct map_info *mapp;
	int found;
	char *p,*p2;

//	dprintf("name: %s\n", name);
	found = 0;
	list_reset(file_map);
	while((mapp = list_get_next(file_map)) != 0) {
//		dprintf("mapp->name: %s\n", mapp->name);
		if (strcmp(mapp->name,name) == 0) {
//			dprintf("newname: %s\n", mapp->newname);
			found=1;
			break;
		}
	}
	p = (found ? mapp->newname : name);
//	dprintf("p: %s\n", p);

	found = 0;
	list_reset(files);
	while((p2 = list_get_next(files)) != 0) {
		if (strcmp(p,p2) == 0) {
			found=1;
			break;
		}
	}
	if (found) return 0;

	list_reset(dep_map);
	while((mapp = list_get_next(dep_map)) != 0) {
//		dprintf("mapp->name: %s\n", mapp->name);
		if (strcmp(mapp->name,name) == 0) {
//			dprintf("dep name: %s\n", mapp->newname);
			addfile(mapp->newname);
		}
	}

//	dprintf("adding: %s\n", p);
	list_add(files,p,strlen(p)+1);
	return 0;
}

unsigned short get_types(char *line, char *caps) {
	register char *p;
	unsigned short mask, types;
	int i,j;

//	dprintf("****************\n");
	mask = 1;
	types = 0;
	for(i=types_start; i < types_end; i++) {
//		dprintf("mask: 0x%04x\n", mask);
		p = &line[i];
//		dprintf("%d: p: %c\n", i, *p);
		for(j=0; j < strlen(caps); j++) {
//			dprintf("caps[%d]: %c\n", j, caps[j]);
			if (*p == caps[j]) {
//				dprintf("Found.\n");
				types |= mask;
				break;
			}
		}
		mask <<= 1;
	}
//	dprintf("types: 0x%04x\n", types);
	return types;
}

char *type2name(char type) {
	char *t = 0;

	switch(type) {
	case 'D':
		t = "disk";
		break;
	case 'T':
		t = "tape";
		break;
	case 'R':
		t = "cd";
		break;
	case 'M':
		t = "changer";
		break;
	}
	return t;
}

int copy_contents(FILE *fp, char *name) {
	FILE *fp2;

	fp2 = fopen(name,"r");
	if (!fp2) {
#if CREATE_MISSING_FILES
		fprintf(fp2,"static int %s(struct vscsi_device *dp, struct scsi_cmnd *scp) {\n",p);
		fprintf(fp2,"\treturn check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);\n");
		fprintf(fp2,"}\n\n");
#else
		char msg[1024];
		sprintf(msg,"fopen(r) %s",name);
		perror(msg);
		return 1;
#endif
	}
	while(fgets(line,sizeof(line),fp2)) 
		fprintf(fp,"%s",line);
	fclose(fp2);

	return 0;
}

char *fixdesc(char *desc) {
	register char *ptr;

//	dprintf("**************************\n");
//	dprintf("desc: %s\n", desc);

	/* Replace all non-alnum chars with underscore & lower it */
	for(ptr = desc; *ptr; ptr++) {
		if (isalnum((int)*ptr) == 0 && *ptr != '.')
			*ptr = '_';
		else
			*ptr = tolower(*ptr);
	}
//	dprintf("desc: %s\n", desc);

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

//	dprintf("desc: %s\n",desc);
	return desc;
}

int main(int argc, char **argv) {
	FILE *fp;
	char *ops[256], types[16], caps[5];
	int i,j,err;
	unsigned short mask,types_wanted;
	struct op_info newop, *opp;
	struct map_info *mapp;
	register char *p;

	if (argc < 2) {
		printf("usage: mkops <TYPES> [CAPS]\n");
		return 1;
	}

	/* Get types */
	memset(types,0,sizeof(types));
	p = argv[1];
	j = 0;
	for(i=0; i < strlen(p); i++) {
		if (!strchr(types,p[i])) {
			types[j++] = p[i];
			if (j > sizeof(types)-1) {
				printf("too many types specified.\n");
				return 1;
			}
		}
	}
//	printf("types: %s\n", types);

	/* Get caps */
	if (argc > 2) {
		memset(caps,0,sizeof(caps));
		p = argv[2];
		j = 0;
		for(i=0; i < strlen(p); i++) {
			if (!strchr(caps,p[i])) {
				caps[j++] = p[i];
				if (j > sizeof(caps)-1) {
					printf("too many caps specified.\n");
					return 1;
				}
			}
		}
	} else {
		strcpy(caps,"M");
	}
//	printf("caps: %s\n", caps);

	/* Check requested types to see if they're supported */
	err = 0;
	for(i=0; i < strlen(types); i++) {
		if (!type2name(types[i])) {
			printf("invalid type: %c\n", types[i]);
			err++;
		}
	}
	if (err) return 1;

	func_map = list_create();
	if (read_map(func_map,"func")) return 1;
	dprintf("func_map count: %d\n", list_count(func_map));
	file_map = list_create();
	if (read_map(file_map,"file")) return 1;
	dprintf("file_map count: %d\n", list_count(file_map));
	dep_map = list_create();
	if (read_map(dep_map,"dep")) return 1;
	dprintf("dep_map count: %d\n", list_count(dep_map));

	files = list_create();

	fp = fopen("op-num.txt","r");
	if (!fp) {
		perror("fopen(r) op-num.txt");
		return 1;
	}

	/* Skip past header */
	while(fgets(line,sizeof(line),fp)) {
		if (strncmp(line,"--",2) == 0)
			break;
		strcpy(temp,line);
	}
	if (strncmp(line,"--",2) != 0) {
		printf("mkops: unable to find header!\n");
		fclose(fp);
		return 1;
	}
	dprintf("temp: %s\n", temp);
	if (strncmp(temp,"OP",2) != 0) {
		printf("mkops: unable to find header!\n");
		fclose(fp);
		return 1;
	}
	i = 3;

	/* Find the start of the types */
	while(i < strlen(temp) && isspace(temp[i])) i++;
	if (i == strlen(temp)) {
		printf("mkops: unable to get types start!\n");
		fclose(fp);
		return 1;
	}
	types_start = i;
	dprintf("types_start: %d\n", types_start);

	/* Get the length of the types */
	while(i < strlen(temp) && !isspace(temp[i])) i++;
	if (i == strlen(temp)) {
		printf("mkops: unable to get types length!\n");
		fclose(fp);
		return 1;
	}
	types_end = i;
	dprintf("types_end: %d\n", types_end);

	/* Find the start of the description */
	while(i < strlen(temp) && isspace(temp[i])) i++;
	if (i == strlen(temp)) {
		printf("mkops: unable to get desc start!\n");
		fclose(fp);
		return 1;
	}
	desc_start = i;
	dprintf("desc_start: %d\n", desc_start);

	allops = list_create();

	/* Create a bitmask for each type we want */
	mask = 1;
	types_wanted = 0;
	for(i=types_start; i < types_end; i++) {
//		printf("type: %c\n", temp[i]);
		for(j=0; j < strlen(types); j++) {
			if (types[j] == temp[i])
				types_wanted |= mask;
		}
		mask <<= 1;
	}
//	printf("types_wanted: 0x%04x\n", types_wanted);

//	memset(ops,0,sizeof(ops));
	while(fgets(line,sizeof(line),fp)) {
		line[strlen(line)-1] = 0;
//		printf("line: %s\n", line);
		line[2] = 0;
		sprintf(temp,"0x%s", line);
//		printf("temp: %s\n", temp);
		newop.op = strtol(temp,NULL,16);
//		printf("op: %d\n", op);

		newop.types = get_types(line,caps);
		if (!newop.types) continue;

//		dprintf("types: 0x%04x, wanted: 0x%04x\n", newop.types, types_wanted);
		if ((newop.types & types_wanted) == 0) continue;

		strcpy(temp,&line[desc_start]);
		if (newop.op == 127)
			strcpy(temp,"variable_length_cdb");
		else
			fixdesc(temp);
		strcpy(newop.name,temp);
//		dprintf("*** ADDING: op: %02x, types: 0x%04x, func: %s\n", newop.op, newop.types, newop.name);
		list_add(allops, &newop, sizeof(newop));

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
			newop.op = strtol(temp,NULL,16);
			line[7] = 0;
			sprintf(temp,"0x%s", &line[3]);
			newop.types = strtol(temp,NULL,16);
			strcpy(newop.name,&line[8]);
//			dprintf("*** ADDING: op: %02x, types: 0x%04x, func: %s\n", newop.op, newop.types, newop.name);
			list_add(allops, &newop, sizeof(newop));
		}
		fclose(fp);
	}

	/* Build file list */
	list_reset(allops);
	while((opp = list_get_next(allops)) != 0) {
		if (addfile(opp->name))
			return 1;
	}

	/* Create ops.c */
	fp = fopen("ops.c","w+");
	if (!fp) {
		perror("fopen(w+) ops.c");
		return 1;
	}

	/* Include each source */
	list_reset(files);
	while((p = list_get_next(files)) != 0) {
		sprintf(temp,"%s.c",p);
//		printf("temp: %s\n", temp);
		copy_contents(fp,temp);
	}

	/* Build op table */
	fprintf(fp,"\n");
	for(i=0; i < strlen(types); i++) {
		/* Build a ops array for this type */
		memset(ops, 0, sizeof(ops));
#if 0
		for(j=0; j < 256; j++) {
			/* This isnt very efficient :/ */
			list_reset(allops);
			while((opp = list_get_next(allops)) != 0) {
				if (opp->op == j && opp->types & types_wanted) {
					p = opp->name;
					list_reset(func_map);
					while((mapp = list_get_next(func_map)) != 0) {
						if (strcmp(mapp->name,opp->name) == 0) {
							p = mapp->newname;
							break;
						}
					}
					ops[j] = p;
					break;
				}
			}
		}
#endif
		list_reset(allops);
		while((opp = list_get_next(allops)) != 0) {
			dprintf("op: %x, types: %x, name: %s\n", opp->op, opp->types, opp->name);
			if (opp->types & types_wanted) {
				p = opp->name;
				list_reset(func_map);
				while((mapp = list_get_next(func_map)) != 0) {
					if (strcmp(mapp->name,opp->name) == 0) {
						p = mapp->newname;
						break;
					}
				}
				ops[opp->op] = p;
			}
		}

		/* Write out the ops array */
		fprintf(fp,"static vscsi_ops_t vscsi_%s_ops[256] = {\n",type2name(types[i]));
#if DEBUG_OPS
		for(j=0; j < 256; j++) {
			if (ops[j]) 
				fprintf(fp,"\t/* %02x */ { \"%s\",%s },\n", j, ops[j], ops[j]);
			else
				fprintf(fp,"\t/* %02x */ { 0, 0 },\n", j);
		}
#else
		for(j=0; j < 256; j++)
			fprintf(fp,"\t/* %02x */ %s,\n", j, (ops[j] ? ops[j] : "0"));
#endif
		fprintf(fp,"};\n\n");
	}

	/* Write the call func */
	fprintf(fp,"static int vscsi_call_op(struct vscsi_device *dp, struct scsi_cmnd *scp) {\n");
	fprintf(fp,"\tint r;\n");
#if DEBUG_OPS
	fprintf(fp,"\tstruct vscsi_op *op = &dp->ops[scp->cmnd[0]];\n\n");
	fprintf(fp,"\tif (op->name) {\n");
	fprintf(fp,"\t\tdprintk(\"> enter %%s\\n\", op->name);\n");
	fprintf(fp,"\t\tr = op->func(dp, scp);\n");
	fprintf(fp,"\t\tdprintk(\"< exit %%s\\n\", op->name);\n");
#else
	fprintf(fp,"\tif (dp->ops[scp->cmnd[0]]) {\n");
	fprintf(fp,"\t\tr = dp->ops[scp->cmnd[0]](dp, scp);\n");
#endif
	fprintf(fp,"\t} else {\n");
	fprintf(fp,"\t\tr = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);\n");
	fprintf(fp,"\t}\n");
	fprintf(fp,"\treturn r;\n");
	fprintf(fp,"}\n");

	fclose(fp);

	/* Create ops.h */
	fp = fopen("ops.h","w+");
	if (!fp) {
		perror("fopen ops.h");
		return 1;
	}
	fprintf(fp,"struct vscsi_device;\n");
	fprintf(fp,"typedef int (*vscsi_opfunc_t)(struct vscsi_device *, struct scsi_cmnd *);\n");
#if DEBUG_OPS
	fprintf(fp,"struct vscsi_op {\n");
	fprintf(fp,"\tchar *name;\n");
	fprintf(fp,"\tvscsi_opfunc_t func;\n");
	fprintf(fp,"};\n");
	fprintf(fp,"typedef struct vscsi_op vscsi_ops_t;\n");
#else
	fprintf(fp,"typedef vscsi_opfunc_t vscsi_ops_t;\n");
#endif
	for(i=0; i < strlen(types); i++) {
		p = type2name(types[i]);
		if (!p) continue;
		for(j=0; p[j]; j++) temp[j] = toupper(p[j]);
		temp[j] = 0;
		if (!strlen(temp)) continue;
		fprintf(fp,"#define HAVE_%s 1\n",temp);
		fprintf(fp,"static vscsi_ops_t vscsi_%s_ops[];\n",p);
	}
	fclose(fp);

	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "/usr/src/mmtools/libos/libos.h"

#define NAME "DVD-RW"

#define MS 1

#if MS
#include "ms.h"
#endif

char temp[1024];

#if 0
static char *getstr(unsigned char *p, int len) {
	static char string[64];
	int i;

	memcpy(string,p,len);
	string[len] = 0;
#if MS == 0
	for(i=0; i < len; i++) {
		if (string[i] == 0x20) {
			string[i] = 0;
			break;
		}
	}
#endif
	return string;
}
#endif

#if 0
static void hexdump(FILE *fp, unsigned char *data, int len, int br) {
	char line[80],*p;
	int x,y;
	int s,e;

	p = line;
	line[0] = 0;

	if (br) fprintf(fp,"\t{\n");

	s = 0;
	for(x=0; x < len; x++) {
		if ((x % 8) == 0 && x) {
			e = x-1;
			if (br) fprintf(fp,"\t");
			fprintf(fp,"\t%s \t/* %02x - %02x */\n",line,s,e);
			line[0] = 0;
			p = line;
			s = x;
		}
		p += sprintf(p,"0x%02x,", *data);
		data++;
	}
	e = x-1;
	while((x % 8) != 0) {
		p += sprintf(p,"     ");
		x++;
	}
	if (br) fprintf(fp,"\t");
	if (e-s > 0)
		fprintf(fp,"\t%s\t/* %02x - %02x */\n", line,s,e);
	else
		fprintf(fp,"\t%s\t/* %02x */\n", line,s);

	if (br) fprintf(fp,"\t},\n");
}
#endif

void write_pages(FILE *out, char *name, char *type, list l) {
	FILE *in;
	char temp2[256],temp3[256],*p;
	list pl;
	int len;

	sprintf(temp2,"%s_%s_",name,type);

	pl = list_create();

	/* Write pages */
	list_reset(l);
	while((p = list_get_next(l)) != 0) {
		if (strstr(p,temp2)) {
			strcpy(temp3,fixfname(p));
			printf("p: %s, temp3: %s\n", p, temp3);
			fprintf(out,"static unsigned char %s[] = {\n", temp3);
			in = fopen(p,"rb");
			if (!in) continue;
			len = fread(temp,1,sizeof(temp),in);
			printf("len: %d\n", len);
			fclose(in);
			if (len < 1) continue;
			hexdump(out,temp,len,0);
			fprintf(out,"};\n\n");
			list_add(pl,temp3,strlen(temp3)+1);
		}
	}

	fprintf(out,"static struct rom_page %s_%s_pages[] = {\n",fixfname(name),type);
	list_reset(pl);
	while((p = list_get_next(pl)) != 0)
		fprintf(out,"\t%s,\n",p);
	fprintf(out,"\tPAGE_LIST_END\n");
	fprintf(out,"};\n\n");
	list_destroy(pl);
}

int main(void) {
	FILE *in,*out;
	DIR *dirp;
	struct dirent *ent;
	list l;
	int len;
	char *name = NAME;
	char fixed_name[256];

	l = list_create();

	dirp = opendir(".");
	if (!dirp) {
		perror("opendir");
		return 1;
	}
	while((ent = readdir(dirp)) != 0) {
		if (strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
			continue;
		if (strncmp(ent->d_name,name,strlen(name)) == 0) {
//			printf("name: %s\n", ent->d_name);
			list_add(l,ent->d_name,strlen(ent->d_name)+1);
		}
	}
	closedir(dirp);

	strcpy(fixed_name,fixfname(name));

	strcpy(temp,fixfname(name));
	strcat(temp,".c");
	printf("outfile: %s\n", temp);
	out = fopen(temp,"w+");
	if (!out) {
		perror("fopen outfile");
		return 1;
	}
	fprintf(out,"\n");

	/* Write standard page */
	strcpy(temp,name);
	strcat(temp,"_std");
	in = fopen(temp,"rb");
	if (!in) {
		perror("fopen std");
		return 1;
	}
	len = fread(temp,1,sizeof(temp),in);
	fclose(in);
	fprintf(out,"static unsigned char %s_std[] = {\n",fixed_name);
	hexdump(out,fixed_name,len,0);
	fprintf(out,"};\n\n");

	write_pages(out,name,"vpd",l);
	write_pages(out,name,"mode",l);

	fprintf(out,"struct rom_info %s_info = {\n",fixed_name);
	fprintf(out,"\t&%s_std,\n",fixed_name);
	fprintf(out,"\t%s_vpd_pages,\n",fixed_name);
	fprintf(out,"\t%s_mode_pages,\n",fixed_name);
	fprintf(out,"};\n");

	fclose(out);
	return 0;
}


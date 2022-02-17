
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "/usr/src/mmtools/libos/libos.h"

#define BINONLY 0

char temp[1024];

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

void write_std(FILE *fp, char *name, unsigned char *data, int len) {
	fprintf(fp,"static struct standard_page %s_std = {\n", name);
	fprintf(fp,".PQ\t\t\t= 0x%02x,\n", (data[0] & 0xE0) >> 5);
	fprintf(fp,".TYPE\t\t\t= 0x%02x,\n",(data[0] & 0x1F));
	fprintf(fp,".RMB\t\t\t= %d,\n",(data[1] & 0x80) != 0);
	fprintf(fp,".VERSION\t\t= 0x%02x,\n",data[2]);
	fprintf(fp,".NORMACA\t\t= %d,\n",(data[3] & 0x20) != 0);
	fprintf(fp,".HISUP\t\t\t= %d,\n",(data[3] & 0x10) != 0);
	fprintf(fp,".RESPONSE_FORMAT\t= %d,\n",data[3] & 0x0F);
//	fprintf(fp,".ADDITIONAL_LENGTH\t= %d,\n", data[4]);
	fprintf(fp,".SCCS\t\t\t= %d,\n",(data[5] & 0x80) != 0);
	fprintf(fp,".ACC\t\t\t= %d,\n",(data[5] & 0x40) != 0);
	fprintf(fp,".TPGS\t\t\t= 0x%02x,\n",(data[5] >> 4) & 0x03);
	fprintf(fp,"._3PC\t\t\t= %d,\n",(data[5] & 0x08) != 0);
	fprintf(fp,".PROTECT\t\t= %d,\n",(data[5] & 0x01) != 0);
	fprintf(fp,".ENCSERV\t\t= %d,\n",(data[6] & 0x40) != 0);
	fprintf(fp,".VS\t\t\t= %d,\n",(data[6] & 0x20) != 0);
	fprintf(fp,".MULTIP\t\t\t= %d,\n",(data[6] & 0x10) != 0);
	fprintf(fp,".ADDR16\t\t\t= %d,\n",(data[6] & 0x01) != 0);
	fprintf(fp,".WBUS16\t\t\t= %d,\n",(data[7] & 0x20) != 0);
	fprintf(fp,".SYNC\t\t\t= %d,\n",(data[7] & 0x10) != 0);
	fprintf(fp,".CMDQUE\t\t\t= %d,\n",(data[7] & 0x02) != 0);
	fprintf(fp,".VS2\t\t\t= %d,\n",(data[7] & 0x01) != 0);
	fprintf(fp,".VENDOR\t\t\t= \"%s\",\n",getstr(&data[8],8));
	fprintf(fp,".PRODUCT\t\t= \"%s\",\n",getstr(&data[16],16));
	fprintf(fp,".REVISION\t\t= \"%s\",\n",getstr(&data[32],4));
	if (data[4] > 30) {
		fprintf(fp,".SPECIFIC\t\t=\n");
		hexdump(fp,&data[36],20,1);
	} else {
		memset(temp,0,20);
		hexdump(fp,temp,20,1);
	}
	if (data[4] > 50) {
		fprintf(fp,".IUS\t\t\t= %d,\n",(data[56] & 0x01) != 0);
		fprintf(fp,".QAS\t\t\t= %d,\n",(data[56] & 0x02) != 0);
		fprintf(fp,".CLOCKING\t\t= 0x%02x,\n",(data[56] >> 2) & 0x03);
	} else {
		fprintf(fp,".IUS\t\t\t= %d,\n",0);
		fprintf(fp,".QAS\t\t\t= %d,\n",0);
		fprintf(fp,".CLOCKING\t\t= 0x%02x,\n",0);
	}
	if (data[4] > 52) {
		fprintf(fp,".VERSION1\t\t= 0x%04x,\n",(data[58] << 8) | data[59]);
		fprintf(fp,".VERSION2\t\t= 0x%04x,\n",(data[60] << 8) | data[61]);
		fprintf(fp,".VERSION3\t\t= 0x%04x,\n",(data[62] << 8) | data[63]);
		fprintf(fp,".VERSION4\t\t= 0x%04x,\n",(data[64] << 8) | data[65]);
		fprintf(fp,".VERSION5\t\t= 0x%04x,\n",(data[66] << 8) | data[67]);
		fprintf(fp,".VERSION6\t\t= 0x%04x,\n",(data[68] << 8) | data[69]);
		fprintf(fp,".VERSION7\t\t= 0x%04x,\n",(data[70] << 8) | data[71]);
		fprintf(fp,".VERSION8\t\t= 0x%04x,\n",(data[72] << 8) | data[73]);
	} else {
		fprintf(fp,".VERSION1\t\t= 0x%04x,\n",0);
		fprintf(fp,".VERSION2\t\t= 0x%04x,\n",0);
		fprintf(fp,".VERSION3\t\t= 0x%04x,\n",0);
		fprintf(fp,".VERSION4\t\t= 0x%04x,\n",0);
		fprintf(fp,".VERSION5\t\t= 0x%04x,\n",0);
		fprintf(fp,".VERSION6\t\t= 0x%04x,\n",0);
		fprintf(fp,".VERSION7\t\t= 0x%04x,\n",0);
		fprintf(fp,".VERSION8\t\t= 0x%04x,\n",0);
	}
	printf("data[4]: %d\n", data[4]);
	if (data[4] >= 60) {
		fprintf(fp,".ADDL_SPECIFIC\t\t=\n");
		hexdump(fp,&data[74],len-60,1);
	}
	fprintf(fp,"};\n\n");
}

static void get_vpd_desc(char *desc, int num) {
	char *p;

#if 0
01h to 7Fh ASCII Information 7.7.2 Optional
89h ATA Information SAT-2 Optional a
83h Device Identification 7.7.3 Mandatory
86h Extended INQUIRY Data 7.7.4 Optional
85h Management Network Addresses 7.7.5 Optional
87h Mode Page Policy 7.7.6 Optional
81h Obsolete 3.3.7
82h Obsolete 3.3.7
88h SCSI Ports 7.7.7 Optional
90h Protocol Specific Logical Unit Information 7.7.8 Protocol specific b
91h Protocol Specific Port Information 7.7.9 Protocol specific b
84h Software Interface Identification 7.7.10 Optional
00h Supported VPD Pages 7.7.11 Mandatory
80h Unit Serial Number 7.7.12 Optional
8Ah to 8Fh Reserved
92h to AFh Reserved
B0h to BFh (See specific device type)
C0h to FFh Vendor specific
#endif
	switch(num) {
	case 0x00:
		p = "Supported VPD Pages";
		break;
	case 0x01 ... 0x7F:
		p = "ASCII Information";
		break;
	case 0x80:
		p = "Unit Serial Number";
		break;
	case 0x83:
		p = "Device Identification";
		break;
	case 0x84:
		p = "Software Interface Identification";
		break;
	case 0x85:
		p = "Management Network Addresses";
		break;
	case 0x86:
		p = "Extended INQUIRY Data";
		break;
	case 0x87:
		p = "Mode Page Policy";
		break;
	case 0x88:
		p = "SCSI Ports";
		break;
	case 0x89:
		p = "ATA Information";
		break;
	case 0xB0:
		p = "Block Limits";
		break;
	case 0xB1:
		p = "Block Device Characteristics";
		break;
	case 0xC0 ... 0xFF:
		p = "Vendor specific";
		break;
	default:
		p = "unknown";
		break;
	}
	strcpy(desc,p);
}

#if 0
00h Vendor-specific 
01h Read-write error recovery parameters 
02h Disconnect-reconnect - allows the initiator to tune SCSI bus performance 
03h Format parameters (direct-access devices) 
03h Measurement units (scanner devices) 
04h Rigid disk geometry parameters 
05h Flexible disk parameters 
06h Optical memory parameters 
07h Verify error recovery parameters 
08h Caching 
09h Peripheral device - transfers vendor-specific data between an initiator and a peripheral interface below the target (obsolete) 
0Ah Control mode - tagged queuing, extended contingent allegiance, asynchronous event notification, and error logging 
0Bh Medium types supported 
0Ch Notch and partition parameters 
0Dh CDROM parameters 
0Eh CDROM audio control (playback) parameters 
0Fh Reserved 
10h Device configuration 
11h Medium partitions (1) 
12h Medium partitions (2) 
13h Medium partitions (3) 
14h Medium partitions (4) 
15h Extended 
16h Extended device-type specific 
17h Reserved 
18h Protocol-specific LUN 
19h Protocol-specific Port 
1Ah Power conditions page 
1Bh Reserved 
1Ch Informational exceptions control 
1Dh medium-changer element address assignment 
1Eh medium-changer transport geometry parameters 
1Fh medium-changer device capabilities 
20h-3Eh Vendor-specific 
3Fh return all mode pages (valid only for a Mode Sense command) 
#endif
static void get_mode_desc(char *desc, int num) {
	char *p;

	switch(num) {
	case 0x01:
		p = "Read-Write Error Recovery";
		break;
	case 0x02:
		p = "Disconnect-Reconnect";
		break;
	case 0x03:
		p = "Format Device";
		break;
	case 0x04:
		p = "Rigid Disk Geometry";
		break;
	case 0x05:
		p = "Flexible Disk";
		break;
	case 0x07:
		p = "Verify Error Recovery";
		break;
	case 0x08:
		p = "Caching";
		break;
	case 0x09:
		p = "Peripheral device";
		break;
	case 0x0A:
		p = "Control";
		break;
	case 0x0B:
		p = "Medium Types Supported";
		break;
	case 0x0C:
		p = "Notch And Partition";
		break;
	case 0x10:
		p = "XOR Control";
		break;
	case 0x14:
		p = "Enclosure Services Management";
		break;
	case 0x15:
		p = "Extended";
		break;
	case 0x16:
		p = "Extended Device-Type Specific";
		break;
	case 0x18:
		p = "Protocol Specific Logical Unit";
		break;
	case 0x19:
		p = "Protocol Specific Port";
		break;
	case 0x1A:
		p = "Power Condition";
		break;
	case 0x1C:
		p = "Informational Exceptions Control";
		break;
	case 0x20 ... 0x3E:
		p = "Vendor Specific";
		break;
	default:
		p = "unknown";
		break;
	}
	strcpy(desc,p);
}

struct page_info {
	char desc[64];
	int num;
	char name[32];
};

void write_pages(FILE *out, char *name, char *type, list l) {
	FILE *in;
	char temp2[256],temp3[256],*p,*p2;
	list pl;
	int len;
	struct page_info newinfo,*np;

	sprintf(temp2,"%s_%s_",name,type);

	pl = list_create();

	/* Write pages */
	list_reset(l);
	while((p = list_get_next(l)) != 0) {
		if (strstr(p,temp2)) {
			sprintf(temp,"_%s_",type);
			p2 = strstr(p,temp);
			if (!p2) {
				printf("write_pages: p2 is null! (%s)\n",temp);
				exit(1);
			}
			p2 += strlen(temp);
			sprintf(temp3,"0x%s",p2);
			newinfo.num = strtol(temp3,0,16);
			if (strcmp(type,"vpd") == 0)
				get_vpd_desc(newinfo.desc,newinfo.num);
			else
				get_mode_desc(newinfo.desc,newinfo.num);

			strcpy(temp3,fixfname(p));
			strcpy(newinfo.name,temp3);

			fprintf(out,"static unsigned char %s[] = {\n", temp3);
			in = fopen(p,"rb");
			if (!in) continue;
			len = fread(temp,1,sizeof(temp),in);
			fclose(in);
			if (len < 1) continue;
			hexdump(out,temp,len,0);
			fprintf(out,"};\n\n");

			list_add(pl,&newinfo,sizeof(newinfo));
		}
	}

	fprintf(out,"static struct rom_page %s_%s_pages[] = {\n",fixfname(name),type);
	list_reset(pl);
	while((np = list_get_next(pl)) != 0)
		fprintf(out,"\tROM_PAGE(\"%s\",0x%02x,%s),\n",np->desc, np->num, np->name);
	fprintf(out,"\tPAGE_LIST_END\n");
	fprintf(out,"};\n\n");
	list_destroy(pl);
}

int main(int argc, char **argv) {
	FILE *in,*out;
	DIR *dirp;
	struct dirent *ent;
	list l;
	int len;
	char *name,fixed_name[256];

	if (argc < 2) {
		printf("usage: %s <bin prefix>\n",argv[0]);
		return 1;
	}
	name = argv[1];

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
	list_sort(l,0,0);

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
#if BINONLY
	fprintf(out,"static unsigned char %s_std[] = {\n",fixed_name);
	hexdump(out,fixed_name,len,0);
	fprintf(out,"};\n\n");
#else
	write_std(out,fixed_name,temp,len);
#endif

	write_pages(out,name,"vpd",l);
	write_pages(out,name,"mode",l);

	fprintf(out,"struct rom_info %s_info = {\n",fixed_name);
	fprintf(out,"\t512,\n");
	fprintf(out,"\t&%s_std,\n",fixed_name);
	fprintf(out,"\t%s_vpd_pages,\n",fixed_name);
	fprintf(out,"\t%s_mode_pages,\n",fixed_name);
	fprintf(out,"};\n");

	fclose(out);
	return 0;
}


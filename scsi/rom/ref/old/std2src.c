
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MS 0
#include "ms.h"

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

void write_ms(FILE *fp, char *name, unsigned char *data) {
	PINQUIRYDATA p = (PINQUIRYDATA) data;

	fprintf(fp,"INQUIRYDATA %s = {\n",name);
	fprintf(fp,".DeviceType = 0x%02x,\n", p->DeviceType);
	fprintf(fp,".DeviceTypeQualifier = 0x%02x,\n", p->DeviceTypeQualifier);
	fprintf(fp,".DeviceTypeModifier = 0x%02x,\n", p->DeviceTypeModifier);
	fprintf(fp,".RemovableMedia = 0x%02x,\n", p->RemovableMedia);
	fprintf(fp,".Version.ANSI = 0x%02x,\n", p->Version.ANSI);
	fprintf(fp,".Version.ECMA = 0x%02x,\n", p->Version.ECMA);
	fprintf(fp,".Version.ISO = 0x%02x,\n", p->Version.ISO);
	fprintf(fp,".ResponseDataFormat = 0x%02x,\n", p->ResponseDataFormat);
	fprintf(fp,".HiSupport = 0x%02x,\n", p->HiSupport);
	fprintf(fp,".NormACA = 0x%02x,\n", p->NormACA);
	fprintf(fp,".TerminateTask = 0x%02x,\n", p->TerminateTask);
	fprintf(fp,".AERC = 0x%02x,\n", p->AERC);
	fprintf(fp,".AdditionalLength = 0x%02x,\n", p->AdditionalLength);
	fprintf(fp,".Reserved = 0x%02x,\n", p->Reserved);
	fprintf(fp,".Addr16 = 0x%02x,\n", p->Addr16);
	fprintf(fp,".Addr32 = 0x%02x,\n", p->Addr32);
	fprintf(fp,".AckReqQ = 0x%02x,\n", p->AckReqQ);
	fprintf(fp,".MediumChanger = 0x%02x,\n", p->MediumChanger);
	fprintf(fp,".MultiPort = 0x%02x,\n", p->MultiPort);
	fprintf(fp,".ReservedBit2 = 0x%02x,\n", p->ReservedBit2);
	fprintf(fp,".EnclosureServices = 0x%02x,\n", p->EnclosureServices);
	fprintf(fp,".ReservedBit3 = 0x%02x,\n", p->ReservedBit3);
	fprintf(fp,".SoftReset = 0x%02x,\n", p->SoftReset);
	fprintf(fp,".CommandQueue = 0x%02x,\n", p->CommandQueue);
	fprintf(fp,".TransferDisable = 0x%02x,\n", p->TransferDisable);
	fprintf(fp,".LinkedCommands = 0x%02x,\n", p->LinkedCommands);
	fprintf(fp,".Synchronous = 0x%02x,\n", p->Synchronous);
	fprintf(fp,".Wide16Bit = 0x%02x,\n", p->Wide16Bit);
	fprintf(fp,".Wide32Bit = 0x%02x,\n", p->Wide32Bit);
	fprintf(fp,".RelativeAddressing = 0x%02x,\n", p->RelativeAddressing);
	fprintf(fp,".VendorId = \"%s\",\n", getstr(p->VendorId,sizeof(p->VendorId)));
	fprintf(fp,".ProductId = \"%s\",\n", getstr(p->ProductId,sizeof(p->ProductId)));
	fprintf(fp,".ProductRevisionLevel = \"%s\",\n", getstr(p->ProductRevisionLevel,sizeof(p->ProductRevisionLevel)));
	fprintf(fp,".VendorSpecific = \n");
	hexdump(fp,p->VendorSpecific,sizeof(p->VendorSpecific),1);
	fprintf(fp,".IUS = 0x%02x,\n", p->IUS);
	fprintf(fp,".QAS = 0x%02x,\n", p->QAS);
	fprintf(fp,".Clocking = 0x%02x,\n", p->Clocking);
	fprintf(fp,".ReservedBit4 = 0x%02x,\n", p->ReservedBit4);
	fprintf(fp,".Reserved4 = 0x%02x,\n", p->Reserved4);
	fprintf(fp,".Version1 = 0x%02x,\n", p->Version1);
	fprintf(fp,".Version2 = 0x%02x,\n", p->Version2);
	fprintf(fp,".Version3 = 0x%02x,\n", p->Version3);
	fprintf(fp,".Version4 = 0x%02x,\n", p->Version4);
	fprintf(fp,".Version5 = 0x%02x,\n", p->Version5);
	fprintf(fp,".Version6 = 0x%02x,\n", p->Version6);
	fprintf(fp,".Version7 = 0x%02x,\n", p->Version7);
	fprintf(fp,".Version8 = 0x%02x,\n", p->Version8);
	fprintf(fp,".Reserved5 = \n");
	hexdump(fp,p->Reserved5,sizeof(p->Reserved5),1);
	fprintf(fp,"};\n\n");
}

void write_std(FILE *fp, char *name, unsigned char *data) {
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
	fprintf(fp,"};\n\n");
}

int main(int argc, char **argv) {
	FILE *fp;
	unsigned char data[1024];
	char filename[32],name[32],temp[64];
	int i;

	if (argc < 2) {
		printf("usage: std2src <std page binary data file>\n");
		return 1;
	}

	fp = fopen(argv[1],"rb");
	if (!fp) {
		sprintf(temp,"fopen %s", argv[1]);
		perror(temp);
		return 1;
	}
	memset(data,0,sizeof(data));
	fread(&data,sizeof(data),1,fp);
	fclose(fp);
	sprintf(filename,"%s.c", argv[1]);
	fp = fopen(filename,"w+");
	if (!fp) {
		sprintf(temp,"fopen %s\n", name);
		perror(temp);
		return 1;
	}
	strcpy(name,argv[1]);
	if (isdigit(name[0])) {
		name[0] = '_';
		strcpy(&name[1],argv[1]);
	}
	for(i=0; i < strlen(name); i++) {
		if (name[i] == '-')
			name[i] = '_';
	}
	printf("%s\n", name);
#if MS
	write_ms(fp,name,data);
#else
	write_std(fp,name,data);
#endif

	fprintf(fp,"};\n");

	return 0;
}

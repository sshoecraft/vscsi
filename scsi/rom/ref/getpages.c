
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <scsi/sg.h>
#include <scsi/scsi.h>

/* XXX not ready yet */
#define MODE_10 0

int do_ioctl(int fd, unsigned char *cdb, int cdb_len, unsigned char *buffer, int buflen) {
	unsigned char sense_buffer[32];
	sg_io_hdr_t pass;
	int err;

	memset(buffer,0,buflen);
	memset(&pass, 0, sizeof(pass));
	pass.interface_id = 'S';
	pass.dxfer_direction = SG_DXFER_FROM_DEV;
	pass.cmd_len = cdb_len;
	pass.mx_sb_len = sizeof(sense_buffer);
	pass.dxfer_len = buflen;
	pass.dxferp = buffer;
	pass.cmdp = cdb;
	pass.sbp = sense_buffer;
	pass.timeout = 2000;
	err = ioctl(fd, SG_IO, (void *)&pass);
	if (err < 0) {
		perror("ioctl");
		return err;
	}

//	printf("status: %d\n", pass.status);
	return pass.status != 0;
}

static void bindump(unsigned char *data, int bytes) {
        int offset,x,y,end;

        offset = 0;
        for(x=0; x < bytes; x += 16) {
                printf("\t%04X: ",offset);
                end=(x+16 > bytes ? bytes : x+16);
                for(y=x; y < end; y++)  printf("%02X ",data[y]);
                for(y=end; y < x+16; y++) printf("   ");
                printf("  ");
                for(y=x; y < end; y++) {
                        if (data[y] > 32 && data[y] < 127)
                                fputc(data[y],stdout);
                        else
                                fputc('.',stdout);
                }
                fputc('\n',stdout);
                offset += 16;
        }
}

int get_inq_page(int fd, int num, unsigned char *buffer, int buflen) {
	unsigned char cdb[12];
	char name[256];
	int err;

	memset(&cdb,0,sizeof(cdb));
	cdb[3] = buflen / 256;
	cdb[4] = buflen % 256;
//	printf("cdb[4]: %d, cdb[3]: %d\n", cdb[4], cdb[3]);
	if (num < 0) {
		cdb[0] = INQUIRY;
	} else {
		cdb[0] = INQUIRY;
		cdb[1] = 1;
		cdb[2] = num;
	}
	err = do_ioctl(fd, cdb, 6, buffer, buflen);
//	bindump(buffer,buflen);
//	if (err != 0) printf("get_inq_page(%d,%d): %d\n", fd, num, err);
	if (!err) {
		int page_length;

		if (num < 0) {
			page_length = buffer[4] + 5;
			printf("inq page: std, length: %d\n", page_length);
		} else {
			page_length = (buffer[2] << 8 | buffer[3]) + 4;
			printf("inq page: %02x, length: %d\n", num, page_length);
		}
		bindump(buffer,page_length);
	}
	return err;
}

int get_mode_page(int fd, int num, int sub, unsigned char *buffer, int buflen) {
	unsigned char cdb[12];
	int err,cdb_len,i;

	memset(&cdb,0,sizeof(cdb));
#ifdef USE_10
	cdb[0] = 0x5a;
	cdb[2] = num;
	cdb[3] = sub;
	cdb[7] = buflen / 256;
	cdb[8] = buflen % 256;
	cdb_len = 10;
#else
	cdb[0] = 0x1a;
	cdb[2] = num;
	cdb[3] = sub;
	cdb[4] = (buflen > 255 ? 255 : buflen);
	cdb_len = 6;
#endif

	/* Header = 4 bytes 
	0 MODE DATA LENGTH
	1 MEDIUM TYPE
	2 DEVICE-SPECIFIC PARAMETER
	3 BLOCK DESCRIPTOR LENGTH
	*/
	err = do_ioctl(fd, cdb, cdb_len, buffer, buflen);
	if (!err) {
		printf("mode data length: %d\n", buffer[0]);
		bindump(buffer, buffer[0]);
	} else
		perror("get_mode_page ioctl");

	return err;
}

int doit(char *dev) {
	unsigned char buffer[255];
	char name[33],filename[64];
	unsigned char list[16];
	FILE *fp;
	int page_length;
	int fd,i,npages,len;

	printf("dev: %s\n", dev);
	fd = open(dev, O_RDONLY);
	if (fd < 0) {
//		perror("open");
		return 1;
	}
//	printf("fd: %d\n", fd);

	/* Get the std page */
//	printf("%s: getting std page...\n", dev);
	if (get_inq_page(fd,-1,buffer,sizeof(buffer))) return 1;

	/* Derive name from product/vendor */
	memcpy(name,&buffer[16],16);
	name[16] = 0;
	printf("product name: %s\n", name);
	for(i=0; i < 16; i++) {
		if (name[i] == 0x20) {
			name[i] = 0;
			break;
		} else if (name[i] == '-')
			name[i] = '_';
		else
			name[i] = tolower(name[i]);
	}
	if (!strlen(name)) {
		memcpy(name,&buffer[8],8);
		name[8] = 0;
		for(i=0; i < 8; i++) {
			if (name[i] == 0x20) {
				name[i] = 0;
				break;
			}
		}
		if (!strlen(name)) {
			printf("vend/prod empty!\n");
//			return 1;
		}
	}
//	printf("%s: name: %s\n", dev, name);
	if (strcmp(name,"CODISK")==0) return 0;

	/* Write out std page */
	sprintf(filename,"%s_std", name);
	fp = fopen(filename,"wb+");
	if (!fp) {
		sprintf(buffer,"fopen %s", filename);
		perror(buffer);
		return 1;
	}
	fwrite(buffer,buffer[4]+5,1,fp);
	fclose(fp);

	/* Get the list of supported vpd pages */
//	printf("%s: getting vpd page 00...\n", dev);
	if (get_inq_page(fd,0,buffer,sizeof(buffer)) == 0) {

	/* For every page in list ... */
	npages = buffer[3];
//	printf("%s: npages: %d\n", dev, npages);
	memset(list,0,sizeof(list));
	memcpy(list,&buffer[4],npages);
	for(i=0; i < npages; i++) {
	//	if (list[i] == 0) continue;
//		printf("%s: getting vpd page %02x...\n", dev, list[i]);
		if (get_inq_page(fd,list[i],buffer,sizeof(buffer))) continue;
		page_length = (buffer[2] << 8 | buffer[3]) + 3;
//		printf("page_length: %x\n", page_length);
		sprintf(filename,"%s_vpd_%02x",name,list[i]);
		fp = fopen(filename,"wb+");
		if (!fp) {
			sprintf(buffer,"fopen %s", filename);
			perror(buffer);
			return 1;
		}
		fwrite(buffer,page_length,1,fp);
		fclose(fp);
	}
	}

//	printf("%s: getting mode page list...\n", dev);
	if (get_mode_page(fd,0x3f,0,buffer,sizeof(buffer)) == 0) {
		int x,c;
		unsigned char *p;
		int page_code,subpage,spf;

		printf("header:\n");
		bindump(buffer,4);
		printf("desc len: %d\n", buffer[3]);
		i = 4;
		if (buffer[3]) {
			c = buffer[3]/8;
			for(x=0; x < c; x++) {
				printf("block desc %d:\n",x);
				bindump(&buffer[i],8);
				i += 8;
			}
		}
//		printf("i: %d, buffer[0]: %d, buffer[%d]: %02x\n", i, buffer[0], i, buffer[i]);
//		bindump(&buffer[i],4);
		while(i < buffer[0]) {
			spf = ((buffer[i] & 0x40) != 0 ? 1 : 0);
			page_code = buffer[i] & 0x3f;
//			printf("page_code: %02x, spf: %d\n", page_code, spf);
			i++;
			if (spf) {
				subpage = buffer[i++];
				page_length = buffer[i+1] << 8 | buffer[i];
				i += 2;
			} else {
				subpage = 0;
				page_length = buffer[i++];
			}
//			printf("subpage: %d, page_length: %d\n",subpage,page_length);
			printf("mode page: %02x, subpage: %02x, length: %d\n", page_code, subpage, page_length);
			bindump(&buffer[i],page_length);
			sprintf(filename,"%s_mode_%02x",name,page_code);
			fp = fopen(filename,"wb+");
			if (!fp) {
				sprintf(buffer,"fopen %s", filename);
				perror(buffer);
				return 1;
			}
			fwrite(&buffer[i],page_length,1,fp);
			fclose(fp);
			i += page_length;
//			printf("i: %d, buffer[0]: %d\n", i, buffer[0]);
		}
	}

	close(fd);
	return 0;
}

int main(void) {
	int i;
	char name[32];

	for(i=0; i < 10; i++) {
		printf("--------------------------------------------------------------------------\n");
		sprintf(name,"/dev/sg%d", i);
		if (doit(name)) break;
	}
	return 0;
}

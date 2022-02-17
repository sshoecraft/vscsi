
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/* XXX should not be here */
#define SDT_MAX	0x10
#define __HAVE_TABLES
#include "opcodes.h"

struct {
	char status;
	char desc[128];
} opcodes[SDT_MAX][OPCODE_MAX];

static char t_table[32];
static int t_count;

int main(void) {
	FILE *fp,*fp2;
	char line[128],temp[128],*desc;
	int t_start,t_end;
	int o_start,d_start;
	int opcode;
	register char *ptr,*eptr;
	register int x,y;

	fp = fopen("op-num.txt","r");
	if (!fp) {
		perror("fopen op-num.txt for read");
		return 1;
	}

	/* Get the device type and desc start columns */
	printf("get devtype.\n");
	o_start = t_start = d_start = -1;
	t_end = t_count = 0;
	x = 1;
	y = 0;
	while(1) {
		memset(line,0,sizeof(line));
		if (!fgets(line,sizeof(line),fp)) break;
		ptr = line + strlen(line)-1;
		while(*ptr == '\n' || *ptr == '\r' && ptr >= line) ptr--;
		*(ptr+1) = 0;

		/* Convert to uppercase */
		for(ptr=line; *ptr; ptr++) *ptr = toupper(*ptr);

		/* Look for type start */
		if (t_start == -1) {
			ptr = strstr(line,"D - ");
			if (ptr)
				t_start = ptr - line;
			else
				continue;
		}

		/* If here, we have type start, parse loc,char,desc */
		ptr = strstr(line," - ");
		if (ptr) {
			ptr--;
			t_table[t_count++] = temp[0] = *ptr;
			if (ptr-line > t_end) t_end = ptr-line;
			ptr += 4;
			eptr = strchr(ptr,'(');
			if (!eptr) {
				eptr = strstr(ptr,"  ");
				if (!eptr)
					eptr = strchr(ptr,'\0');
				else
					*eptr = '\0';
			}
			else
				*eptr = '\0';
			x <<= 1;
			y++;

			/* Device types 10 and 11 are reserved */
			if (y == 10) {
				y+=2;
				x <<= 2;
			}
		} else if (o_start == -1 || d_start == -1) {
			if (strstr(line,"OP") && 
			    strstr(line,"DTLP") &&
			    strstr(line,"DESC")) {
				ptr = strstr(line,"OP");
				o_start = ptr - line;
				ptr = strstr(line,"DESC");
				d_start = ptr - line;
			}
		} else
			if (strstr(line,"------------------------------------"))
				break;
	}
	if (o_start == -1) {
		printf("Could not fine o_start.\n");
		return 1;
	}
	if (t_start == -1) {
		printf("Could not find t_start.\n");
		return 1;
	}
	if (d_start == -1) {
		printf("Could not find d_start.\n");
		return 1;
	}

	printf("init\n");
	/* Init the opcodes table */
	for(x=0; x < SDT_MAX; x++) {
		for(y=0; y < OPCODE_MAX; y++) {
			opcodes[x][y].status = 'I';
			opcodes[x][y].desc[0] = 0;
		}
	}

	printf("get\n");
	fflush(stdout);
	/* Get the opcode info */
	while(1) {
		memset(line,0,sizeof(line));
		if (!fgets(line,sizeof(line),fp)) break;
		ptr = line + strlen(line)-1;
		while(*ptr == '\n' || *ptr == '\r' && ptr >= line) ptr--;
		*(ptr+1) = 0;

		/* Get the opcode */
		temp[0] = '0';
		temp[1] = 'x';
		temp[4] = 0;
		memcpy(&temp[2],(char *)&line[o_start],2);
		opcode = strtol(temp,0,16);
		if (opcode < 0 || opcode > 255) {
			printf("invalid opcode: %x\n",opcode);
			continue;
		}

		/* Get the description */
		temp[0] = 0;
		ptr = &line[d_start];
		y=0;
		while(*ptr && y < sizeof(temp)-1) temp[y++] = tolower(*ptr++);
		temp[y] = 0;
		temp[0] = toupper(temp[0]);
		printf("temp: %s\n",temp);

		/* Get the types info */
		for(x=t_start; x <= t_end; x++) {
			y=x-t_start;
			if (y >= 10) y+=2;
			switch(line[x]) {
			case 'M':
			case 'O':
			case 'V':
			case 'R':
			case 'Z':
				opcodes[y][opcode].status = line[x];
				if (strlen(temp))
					strcpy(opcodes[y][opcode].desc,temp);
				else if (line[x] == 'V')
					strcpy(opcodes[y][opcode].desc,
						"(vendor specific)");
				break;
#if 0
			default:
				opcodes[y][opcode].status = 'I';
				break;
#endif
			}
#if 0
			if (line[x] != ' ') {
				if (y) *ptr++ = '|';
				*ptr++ = t_table[x-t_start];
				y++;
			}
#endif
		}
	}

	printf("writing table...\n");
	/* Create the opcodes table */
	fp2 = fopen("opcodes.c","w+");
	if (!fp2) {
		perror("fopen opcodes.c for write");
		fclose(fp);
		return 1;
	}
	fprintf(fp2,"\nOPCODE_INFO opcodes[SDT_MAX][OPCODE_MAX] = {\\\n");
	for(x=0; x < SDT_MAX; x++) {
		fprintf(fp2,"{ /* %d */\n",x);
		for(y=0; y < OPCODE_MAX; y++) {
			fprintf(fp2,"\t{ \'%c\', \"%s\" }, /* %02X */\n",
				opcodes[x][y].status,opcodes[x][y].desc,y);
		}
		fprintf(fp2,"},\n");
#if 0
		ptr = opcodes[x].types;
		eptr = opcodes[x].desc;
		if (!strlen(ptr)) ptr = "0";
		if (!strlen(ptr) && !strlen(eptr)) {
			ptr = "0";
			eptr = "(Invalid)";
		} else if (!strlen(ptr) && strlen(eptr))
			ptr = "0";
		else if (strlen(ptr) && !strlen(eptr))
			eptr = "(Vendor-specific)";
#endif
		
	}
	fprintf(fp2,"};\n");
	fclose(fp2);
	fclose(fp);
	return 0;
}

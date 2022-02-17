
#if 0
static void bindump(char *msg, void *bdata, int bytes) {
#if DEBUG
        int offset,end,x,y;
	static char bline[256];
	register unsigned char *data = bdata, *p;

	dprintk("%s:\n",msg);
	p = bline;
        offset = 0;
        for(x=0; x < bytes; x += 16) {
		p += sprintf(p, "%04X: ",offset);
                end=(x+16 > bytes ? bytes : x+16);
                for(y=x; y < end; y++)  p += sprintf(p,"%02X ",data[y]);
                for(y=end; y < x+16; y++) p += sprintf(p,"   ");
                p += sprintf(p,"  ");
                for(y=x; y < end; y++) {
                        if (data[y] > 32 && data[y] < 127)
				p += sprintf(p,"%c",data[y]);
                        else
				p += sprintf(p,".");
                }
		p += sprintf(p,"\n");
		dprintk(bline);
		p = bline;
                offset += 16;
        }
#endif
}
#endif

//#define dump_data(u,s,d,l) bindump(s,d,min(l,64));
#if DEBUG
#define dump_data(u,s,d,l) print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET, 16, 1, d, min(l,256), 1);
//#define dump_data(u,s,d,l) /* noop */
#else
#define dump_data(u,s,d,l) /* noop */
#endif

#if 0
static void dump_data(int unit, char *str, void *data, int data_len) {
//	dprintk(KERN_INFO "scsi%d: %s(%d bytes):\n",unit,str,data_len);
//	bindump(str,data,min(data_len,64));
        print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET, 16, 1, data, min(data_len,16), 1);
#if 0
	char line[128], *lp;
        unsigned char *p;
        int x,y,len,off;

	dprintk(KERN_INFO "scsi%d: %s(%d bytes):\n",unit,str,data_len);
//	len = min(16, data_len);
	len = data_len;
        print_hex_dump(KERN_INFO, "test", DUMP_PREFIX_OFFSET, 32, 1, data, len, 1);
	void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
                        const void *buf, size_t len)
        p = data;
	off = 0;
	sprintf(line,"%04x: ",off);
	lp = line + strlen(line);
        for(x=y=0; x < len; x++) {
		sprintf(lp," %02x",p[x]);
		lp += 3;
                y++;
                if (y > 15) {
                        dprintk(KERN_INFO "%s\n", line);
			off += 16;
			sprintf(line,"%04x: ",off);
			lp = line + strlen(line);
                        y = 0;
                }
        }
        if (y) dprintk(KERN_INFO "%s\n", line);
#endif
	return;
}
#endif

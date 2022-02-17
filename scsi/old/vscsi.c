
#define DEBUG 1

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
#include <linux/platform_device.h>
#endif

#include <asm/system.h>
#include <asm/string.h>
#include <asm/pgtable.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsi_transport.h>
#include <scsi/scsi_transport_spi.h>
#else
#include <linux/blk.h>
#include "scsi.h"
#include "hosts.h"

#if 0
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/genhd.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/smp_lock.h>
#include <linux/vmalloc.h>

#include <asm/io.h>

#include <linux/blk.h>

#include <linux/stat.h>
#endif
#endif

#define VSCSI_VERSION_MAJOR 1
#define VSCSI_VERSION_MINOR 0
#define VSCSI_VERSION "1.00"

#ifdef __arch_um__
#include "os.h"
#endif

MODULE_AUTHOR("Steve Shoecraft <sshoecraft@earthlink.net>");
MODULE_DESCRIPTION("Virtual SCSI Driver " VSCSI_VERSION);
MODULE_LICENSE("GPL");
//MODULE_ALIAS("platform:vscsi");

/* Sense codes */
#define LOGICAL_UNIT_NOT_READY 0x4
#define INVALID_FIELD_IN_CDB 0x24
#define MEDIUM_NOT_PRESENT 0x3a

/* Additional Sense Codes */
#define CAUSE_NOT_REPORTABLE 0x00
#define INITIALIZING_COMMAND_REQUIRED 0x02
#define FORMAT_IN_PROGRESS 0x04
#define LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE 0x21

#ifdef min
#undef min
#endif
#define min(a,b) ((a) < (b) ? (a) : (b))

#ifdef max
#undef max
#endif
#define max(a,b) ((a) < (b) ? (b) : (a))

#define TYPE_PASS 0x1f

//static void host_printf(char *,...);
extern void exit(int);

//#define dprintk host_printf
#if DEBUG
#define dprintk printk
#else
#define dprintk(...) /* noop */
#endif

static unsigned char temp[192];

/* For read/write_buffer */
static unsigned char echobuf[128];

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static struct scsi_transport_template *vscsi_transport_template;
#endif

/* 0 - dont check command, 1 check command */
#define VSCSI_STRICT 0

static void calc_max_lba(unsigned long long size, unsigned long block_size, unsigned long long *mlba) {
	unsigned long mask = 0x80000000;
	int bits;

	dprintk("calc_mlba: size: %lld, block_size: %ld\n", size, block_size);

	/* Calc max lba */
	bits = 31;
	while(mask) {
		if (block_size & mask) break;
		mask >>= 1;
		bits--;
	}
	dprintk("bits: %d\n", bits);
	*mlba = size >> bits;
	dprintk("max lba: %lld\n", *mlba);
//	if (dp->max_lba * sdev->sector_size < dp->fp->size) dp->max_lba++;
//	dprintk("adjusted max lba: %lld\n", dp->max_lba);
}
static void bindump(char *msg, void *bdata, int bytes) {
	unsigned char *data = bdata;
        int offset,end,x,y;
	char line[128],*p;

	dprintk("%s:\n",msg);
	p = line;
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
		dprintk(line);
		p = line;
                offset += 16;
        }
}


#define dump_data(u,s,d,l) bindump(s,d,min(l,64));

#if 0
static void dump_data(int unit, char *str, void *data, int data_len) {
	dprintk(KERN_INFO "scsi%d: %s(%d bytes):\n",unit,str,data_len);
	bindump(data,min(data_len,64));
#if 0
        print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET, 16, 1, data, min(data_len,16), 1);
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


static int response(struct scsi_cmnd *scp, void *data, int data_len) {
	struct scatterlist *sg;
	unsigned char *buffer = 0;
	int buflen = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	struct scsi_data_buffer *sdb = scsi_in(scp);
	int i;

	if (!sdb->length) return 0;
	if (!sdb->table.sgl) return (DID_ERROR << 16);
	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE)) return (DID_ERROR << 16);

	for_each_sg(sdb->table.sgl, sg, sdb->table.nents, i) {
		buffer = sg_virt(sg);
		buflen = sg->length;
		break;
	}
#else
	/* Scatter/Gather */
	if (scp->use_sg) {
		sg = (struct scatterlist *)scp->request_buffer;
		buffer = page_address(sg->page) + sg->offset;
		buflen = sg->length;

	/* Direct */
        } else {
		buffer = scp->request_buffer;
		buflen = scp->request_bufflen;
	}
#endif
	if (!buffer) return (DID_ERROR << 16);
	if (!buflen) return GOOD;

	dump_data(scp->device->id, "reponse", data, min(buflen,data_len));
	memcpy(buffer, data, min(buflen, data_len));

	return GOOD;
}

#if 0
		if (active) {
			kaddr = (unsigned char *) kmap_atomic(sg_page(sg), KM_USER0);
			if (!kaddr) return (DID_ERROR << 16);
			kaddr_off = (unsigned char *)kaddr + sg->offset;
			len = sg->length;
			if ((req_len + len) > data_len) {
				active = 0;
				len = data_len - req_len;
			}
			memcpy(kaddr_off, data + req_len, len);
			kunmap_atomic(kaddr, KM_USER0);
			act_len += len;
		}
		req_len += sg->length;
#endif

struct vscsi_file_ops;

struct vscsi_file {
	void *fd;			/* Host/Local file desc */
	char path[1024];		/* Path to file */
	unsigned long long size;	/* File size, in bytes */
	struct vscsi_file_ops *ops;	/* File ops */
};

/* Local ops */
struct vscsi_file_ops {
	int (*open)(struct vscsi_file *, int);
	int (*close)(struct vscsi_file *);
	int (*rw)(struct vscsi_file *, struct scsi_cmnd *, unsigned long long, int, int);
	int (*pass)(struct vscsi_file *, struct scsi_cmnd *);
};

static int vscsi_local_open(struct vscsi_file *fp, int read_only) {
	dprintk("vscsi_file_open: path: %s\n", fp->path);
	return 1;
}

static int vscsi_local_close(struct vscsi_file *fp) {
	dprintk("vscsi_file_close: closing file...\n");
	return 1;
}

static int vscsi_local_rw(struct vscsi_file *fp, struct scsi_cmnd *scp, unsigned long long lba, int num, int write) {
	return 1;
}

/* Doesn't make sense */
static int vscsi_local_pass(struct vscsi_file *fp, struct scsi_cmnd *scp) {
	return 1;
}

/* UML Host funcs */
#ifdef __arch_um__
#if BITS_PER_LONG == 64
typedef long long fd_type_t;
#else
typedef long fd_type_t;
#endif
#include <scsi/sg.h>

#if 0
/*
 * worker thread that handles reads/writes to file backed loop devices,
 * to avoid blocking in our make_request_fn. it also does loop decrypting
 * on reads for block backed loop, as that is too heavy to do from
 * b_end_io context where irqs may be disabled.
 *
 * Loop explanation:  loop_clr_fd() sets lo_state to Lo_rundown before
 * calling kthread_stop().  Therefore once kthread_should_stop() is
 * true, make_request will not place any more requests.  Therefore
 * once kthread_should_stop() is true and lo_bio is NULL, we are
 * done with the loop.
 */
static int loop_thread(void *data)
{
        struct loop_device *lo = data;
        struct bio *bio;

        set_user_nice(current, -20);

        while (!kthread_should_stop() || lo->lo_bio) {

                wait_event_interruptible(lo->lo_event, lo->lo_bio || kthread_should_stop());

                if (!lo->lo_bio) continue;
                spin_lock_irq(&lo->lo_lock);
                bio = loop_get_bio(lo);
                spin_unlock_irq(&lo->lo_lock);

                BUG_ON(!bio);
                loop_handle_bio(lo, bio);
        }

        return 0;
}
        lo->lo_thread = kthread_create(loop_thread, lo, "loop%d",
                                                lo->lo_number);
        if (IS_ERR(lo->lo_thread)) {
                error = PTR_ERR(lo->lo_thread);
                goto out_clr;
        }

#endif

static int vscsi_host_open(struct vscsi_file *fp, int read_only) {
	struct openflags flags;
	fd_type_t fd;

	/* Open the file */
	dprintk("vscsi_host_open: BITS_PER_LONG: %d\n", BITS_PER_LONG);
	dprintk("vscsi_host_open: path: %s\n", fp->path);
	flags.r = 1;
	if (!read_only) flags.w = 1;
	fd = os_open_file(fp->path, flags, 0);
	dprintk("vscsi_host_open: fd: %lld\n", (long long)fd);
	if (fd < 0) return 1;
	fp->fd = (void *) fd;

	/* Get the size */
	if (os_file_size(fp->path,&fp->size)) {
		dprintk(KERN_WARNING "vscsi_host_open: os_file_size failed!\n");
		os_close_file(fd);
		return 1;
	}
	dprintk("size: %lld\n", fp->size);

	return 0;
}

static int vscsi_host_close(struct vscsi_file *fp) {
	dprintk("vscsi_host_close: fd: %lld\n", (long long)fp->fd);
	os_close_file((fd_type_t)fp->fd);
	fp->size = 0;
	return 0;
}

static int vscsi_host_rw(struct vscsi_file *fp, struct scsi_cmnd *scp, unsigned long long lba, int num, int write) {
	struct scatterlist *sg;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
	struct scsi_data_buffer *sdb = scsi_in(scp);
	int fd = (fd_type_t) fp->fd;
	int (*func)(int, void *, int);
	unsigned long long offset;
	int i,len;

	dprintk("vscsi_host_rw: lba: %lld, num: %d, write: %d\n", lba, num, write);

//	if (write) return 1;

	if (!sdb->length) return 0;
	if (!sdb->table.sgl) return (DID_ERROR << 16);
//	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE)) return (DID_ERROR << 16);

	offset = lba*scp->device->sector_size;
	len = num*scp->device->sector_size;
	dprintk("vscsi_host_rw: offset: %lld, len: %d\n", offset, len);

	func = (write ? os_write_file : os_read_file);

#if 0
/* If the LBA plus the transfer length exceeds the capacity of the medium, then the device server shall terminate the command with CHECK CONDITION status with the sense key set to ILLEGAL REQUEST and the additional sense code set to LOGICAL BLOCK ADDRESS OUT OF RANGE. The TRANSFER LENGTH field is constrained by the MAXIMUM TRANSFER LENGTH field in the Block Limits VPD page (see 6.4.2). */
	len = num * scp->device->sector_size;
	dprintk("vscsi_host_rw: len: %d\n", len);
	if (lba + len > fp->size)
		return check_condition(dp, ILLEGAL_REQUEST, LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE, 0);
#endif

	os_seek_file(fd,offset);
	for_each_sg(sdb->table.sgl, sg, sdb->table.nents, i) {
		if (func(fd,sg_virt(sg),sg->length) < 0) {
			dprintk(KERN_ERR "vscsi_file_read: file read error!\n");
			return 1;
		}
	}
#else
	char *buffer;
	int buflen;

	/* Scatter/Gather */
	if (scp->use_sg) {
		sg = (struct scatterlist *)scp->request_buffer;
		buffer = page_address(sg->page) + sg->offset;
		buflen = sg->length;
	/* Direct */
	} else {
		buffer = scp->request_buffer;
		buflen = scp->request_bufflen;
        }
#endif

	return 0;
}

static int vscsi_host_pass(struct vscsi_file *fp, struct scsi_cmnd *scp) {
#if 0
        struct scatterlist *sg;
        unsigned char *buffer = 0;
        int buflen = 0,dir,result;
	int fd = (fd_type_t) fp->fd;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
        struct scsi_data_buffer *sdb = scsi_in(scp);
        int i;

//	printk("sdb->table.sgl: %p\n", sdb->table.sgl);
//	if (!sdb->table.sgl) return (DID_ERROR << 16);
	printk("scsi_bidi_cmnd(scp): %d\n", scsi_bidi_cmnd(scp));
//	if (!(scsi_bidi_cmnd(scp) || scp->sc_data_direction == DMA_FROM_DEVICE)) return (DID_ERROR << 16);

	if (sdb->table.sgl) {
		for_each_sg(sdb->table.sgl, sg, sdb->table.nents, i) {
			buffer = sg_virt(sg);
			buflen = sg->length;
			break;
		}
	}

#else
        /* Scatter/Gather */
        if (scp->use_sg) {
                sg = (struct scatterlist *)scp->request_buffer;
                buffer = page_address(sg->page) + sg->offset;
                buflen = sg->length;

        /* Direct */
        } else {
                buffer = scp->request_buffer;
                buflen = scp->request_bufflen;
        }
#endif
//        if (!buffer) return (DID_ERROR << 16);
//        if (!buflen) return GOOD;

	if (scp->sc_data_direction == DMA_FROM_DEVICE)
		dir = -1;
	else if (scp->sc_data_direction == DMA_NONE)
		dir = 0;
	else
		dir = 1;
	printk("dir: %d\n", dir);
	result = os_sgio(fd, scp->cmnd, scp->cmd_len, buffer, buflen, dir);
	if (result == 0 && dir < 0) dump_data(0, "pass response", buffer, buflen);
	return result;
#endif
	return 1;
}

#if 0
static void host_printf(char *fmt, ...) {
	char msg[1024];
	va_list ap;

	va_start(ap,fmt);
	vsprintf(msg,fmt,ap);
	va_end(ap);
	os_write_file(2,msg,strlen(msg));
}
#endif

/* coLinux host funcs */
#elif defined(CONFIG_COOPERATIVE)
#define vscsi_host_open vscsi_local_open
#define vscsi_host_close vscsi_local_close
#define vscsi_host_rw vscsi_local_rw
#define vscsi_host_pass vscsi_local_pass

/* No host funcs, host = local */
#else
#define vscsi_host_open vscsi_local_open
#define vscsi_host_close vscsi_local_close
#define vscsi_host_rw vscsi_local_rw
#define vscsi_host_pass vscsi_local_pass

#if 0
static void host_printf(char *fmt, ...) {
	char msg[1024];
	va_list ap;

	va_start(ap,fmt);
	vsprintf(msg,fmt,ap);
	va_end(ap);
	printk(msg);
}
#endif
#endif

static struct vscsi_file_ops vscsi_host_fops = {
	vscsi_host_open,
	vscsi_host_close,
	vscsi_host_rw,
	vscsi_host_pass,
};

static struct vscsi_file_ops vscsi_local_fops = {
	vscsi_local_open,
	vscsi_local_close,
	vscsi_local_rw,
	vscsi_local_pass,
};

static struct vscsi_file *vscsi_file_alloc(char *path, int which) {
	struct vscsi_file *fp;

	dprintk("vscsi_file_alloc: path: %s, which: %d\n", path, which);
	fp = kmalloc(sizeof(struct vscsi_file),GFP_KERNEL);
	if (!fp) return 0;

	strncpy(fp->path, path, sizeof(fp->path)-1);
	fp->ops = (which ? &vscsi_host_fops : &vscsi_local_fops);

	return fp;
}

static void vscsi_file_free(struct vscsi_file *fp) {
	kfree(fp);
}

/* File ops macros */
#define vscsi_file_open(f,r) f->ops->open(f,r)
#define vscsi_file_close(f) f->ops->close(f)
#define vscsi_file_rw(f,s,b,n,w) f->ops->rw(f,s,b,n,w)
#define vscsi_file_pass(f,s) f->ops->pass(f,s)
/* Mode/Inq page data */
struct vscsi_rom_page {
	int num;
	unsigned char *data;
	int size;
};
#define VSCSI_ROM_PAGE(n,p) { n, p, sizeof(p) }

/* ROM Definition */
struct vscsi_rom {
	int sector_size;
	struct vscsi_rom_page std;
	struct vscsi_rom_page *vpd;
	struct vscsi_rom_page *mode;
};

static unsigned char vDISK_std_page[] = {
	0x00,0x00,0x06,0x12,0x5b,0x00,0x01,0x32,	/* 00 - 07 */
	0x76,0x53,0x43,0x53,0x49,0x20,0x20,0x20,	/* 08 - 0f */
	0x76,0x44,0x49,0x53,0x4b,0x20,0x20,0x20,	/* 10 - 17 */
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,	/* 18 - 1f */
	0x31,0x2e,0x30,0x30,0x00,0x00,0x00,0x00,	/* 20 - 27 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 28 - 2f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 30 - 37 */
	0x0e,0x00,0x00,0x40,0x0b,0x54,0x01,0x3c,	/* 38 - 3f */
	0x01,0x9b,0x00,0x00,0x00,0x00,0x00,0x00,	/* 40 - 47 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 48 - 4f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 50 - 57 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 58 - 5f */
};

/* Supported VPD Pages */
static unsigned char vDISK_vpd_00[] = {
	0x00,0x00,0x00,0x03,0x00,0x80,0x83,     	/* 00 - 06 */
};

/* Unit Serial Number VPD Page */
static unsigned char vDISK_vpd_80[] = {
	0x00,0x80,0x00,0x08,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* Device Identification VPD Page */
static unsigned char vDISK_vpd_83[] = {
	0x00,0x83,0x00,0x0c,0x11,0x83,0x00,0x08,	/* 00 - 07 */
	0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
};

/* vDISK Supported VPD Page List */
static struct vscsi_rom_page vDISK_vpd_pages[] = {
	VSCSI_ROM_PAGE(0x00, vDISK_vpd_00),		/* Supported Pages */
	VSCSI_ROM_PAGE(0x80, vDISK_vpd_80),		/* Unit Serial Number */
	VSCSI_ROM_PAGE(0x83, vDISK_vpd_83),		/* Device Identification */
	VSCSI_ROM_PAGE(0,0)
};

/* Caching Mode Page */
static unsigned char vDISK_mode_08[] = {
	0x08,0x12,0x14,0x00,0xff,0xff,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0xff,0xff,0x80,0x08,0x00,0x00,	/* 08 - 0f */
	0x00,0x00,0x00,0x00,                    	/* 10 - 13 */
};

/* Control Mode Page */
static unsigned char vDISK_mode_0a[] = {
	0x02,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* vDISK Supported Mode Page List */
static struct vscsi_rom_page vDISK_mode_pages[] = {
	VSCSI_ROM_PAGE(0x08, vDISK_mode_08),		/* Caching */
	VSCSI_ROM_PAGE(0x0a, vDISK_mode_0a),		/* Control */
	VSCSI_ROM_PAGE(0,0)
};

/* vDISK ROM Definition */
static struct vscsi_rom vDISK_rom = {
	512,
	VSCSI_ROM_PAGE(0, vDISK_std_page),
	vDISK_vpd_pages,
	vDISK_mode_pages
};

static unsigned char vCD_std_page[] = {
	0x05,0x80,0x06,0x02,0x5b,0x00,0x01,0x30,	/* 00 - 07 */
	0x76,0x53,0x43,0x53,0x49,0x20,0x20,0x20,	/* 08 - 0f */
	0x76,0x43,0x44,0x20,0x20,0x20,0x20,0x20,	/* 10 - 17 */
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,	/* 18 - 1f */
	0x31,0x2e,0x30,0x30,0x00,0x00,0x00,0x00,	/* 20 - 27 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 28 - 2f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 30 - 37 */
	0x0e,0x00,0x00,0x14,0x00,0x77,0x03,0x3d,	/* 38 - 3f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 40 - 47 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 48 - 4f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 50 - 57 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 58 - 5f */
};

/* Supported VPD Pages */
static unsigned char vCD_vpd_00[] = {
	0x05,0x00,0x00,0x03,0x00,0x80,0x83,     	/* 00 - 06 */
};

/* Unit Serial Number VPD Page */
static unsigned char vCD_vpd_80[] = {
	0x05,0x80,0x00,0x08,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* Device Identification VPD Page */
static unsigned char vCD_vpd_83[] = {
	0x05,0x83,0x00,0x0c,0x11,0x83,0x00,0x08,	/* 00 - 07 */
	0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
};

/* vCD Supported VPD Page List */
static struct vscsi_rom_page vCD_vpd_pages[] = {
	VSCSI_ROM_PAGE(0x00, vCD_vpd_00),		/* Supported Pages */
	VSCSI_ROM_PAGE(0x80, vCD_vpd_80),		/* Unit Serial Number */
	VSCSI_ROM_PAGE(0x83, vCD_vpd_83),		/* Device Identification */
	VSCSI_ROM_PAGE(0,0)
};

/* Control Mode Page */
static unsigned char vCD_mode_0a[] = {
	0x02,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* Power Condition Mode Page */
static unsigned char vCD_mode_1a[] = {
	0x1a,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* CD/DVD Capabilities and Mechanical Status Mode Page */
static unsigned char vCD_mode_2a[] = {
	0x2a,0x18,0x3f,0x00,0x75,0x7f,0x29,0x00,	/* 00 - 07 */
	0x16,0x00,0x01,0x00,0x02,0x00,0x16,0x00,	/* 08 - 0f */
	0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x01,	/* 10 - 17 */
	0x00,0x00,0x00,                         	/* 18 - 1a */
};

/* vCD Supported Mode Page List */
static struct vscsi_rom_page vCD_mode_pages[] = {
	VSCSI_ROM_PAGE(0x0a, vCD_mode_0a),		/* Control */
	VSCSI_ROM_PAGE(0x1a, vCD_mode_1a),		/* Power Condition */
	VSCSI_ROM_PAGE(0x2a, vCD_mode_2a),		/* CD/DVD Capabilities and Mechanical Status */
	VSCSI_ROM_PAGE(0,0)
};

/* vCD ROM Definition */
static struct vscsi_rom vCD_rom = {
	2048,
	VSCSI_ROM_PAGE(0, vCD_std_page),
	vCD_vpd_pages,
	vCD_mode_pages
};

static unsigned char vCHANGER_std_page[] = {
	0x08,0x80,0x06,0x02,0x5b,0x00,0x00,0x00,	/* 00 - 07 */
	0x76,0x53,0x43,0x53,0x49,0x20,0x20,0x20,	/* 08 - 0f */
	0x76,0x43,0x48,0x41,0x4e,0x47,0x45,0x52,	/* 10 - 17 */
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,	/* 18 - 1f */
	0x31,0x2e,0x30,0x30,0x00,0x00,0x00,0x00,	/* 20 - 27 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 28 - 2f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 30 - 37 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 38 - 3f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 40 - 47 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 48 - 4f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 50 - 57 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 58 - 5f */
};

/* Supported VPD Pages */
static unsigned char vCHANGER_vpd_00[] = {
	0x08,0x00,0x00,0x03,0x00,0x80,0x83,     	/* 00 - 06 */
};

/* Unit Serial Number VPD Page */
static unsigned char vCHANGER_vpd_80[] = {
	0x08,0x80,0x00,0x08,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* Device Identification VPD Page */
static unsigned char vCHANGER_vpd_83[] = {
	0x08,0x83,0x00,0x0c,0x11,0x83,0x00,0x08,	/* 00 - 07 */
	0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
};

/* vCHANGER Supported VPD Page List */
static struct vscsi_rom_page vCHANGER_vpd_pages[] = {
	VSCSI_ROM_PAGE(0x00, vCHANGER_vpd_00),		/* Supported Pages */
	VSCSI_ROM_PAGE(0x80, vCHANGER_vpd_80),		/* Unit Serial Number */
	VSCSI_ROM_PAGE(0x83, vCHANGER_vpd_83),		/* Device Identification */
	VSCSI_ROM_PAGE(0,0)
};

/* Control Mode Page */
static unsigned char vCHANGER_mode_0a[] = {
	0x02,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* vCHANGER Supported Mode Page List */
static struct vscsi_rom_page vCHANGER_mode_pages[] = {
	VSCSI_ROM_PAGE(0x0a, vCHANGER_mode_0a),		/* Control */
	VSCSI_ROM_PAGE(0,0)
};

/* vCHANGER ROM Definition */
static struct vscsi_rom vCHANGER_rom = {
	0,
	VSCSI_ROM_PAGE(0, vCHANGER_std_page),
	vCHANGER_vpd_pages,
	vCHANGER_mode_pages
};

static unsigned char vTAPE_std_page[] = {
	0x01,0x80,0x06,0x02,0x5b,0x00,0x01,0x30,	/* 00 - 07 */
	0x76,0x53,0x43,0x53,0x49,0x20,0x20,0x20,	/* 08 - 0f */
	0x76,0x54,0x41,0x50,0x45,0x20,0x20,0x20,	/* 10 - 17 */
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,	/* 18 - 1f */
	0x31,0x2e,0x30,0x30,0x00,0x00,0x00,0x00,	/* 20 - 27 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 28 - 2f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 30 - 37 */
	0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 38 - 3f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 40 - 47 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 48 - 4f */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 50 - 57 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 58 - 5f */
};

/* Supported VPD Pages */
static unsigned char vTAPE_vpd_00[] = {
	0x01,0x00,0x00,0x03,0x00,0x80,0x83,     	/* 00 - 06 */
};

/* Unit Serial Number VPD Page */
static unsigned char vTAPE_vpd_80[] = {
	0x01,0x80,0x00,0x08,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* Device Identification VPD Page */
static unsigned char vTAPE_vpd_83[] = {
	0x01,0x83,0x00,0x0c,0x11,0x83,0x00,0x08,	/* 00 - 07 */
	0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
};

/* vTAPE Supported VPD Page List */
static struct vscsi_rom_page vTAPE_vpd_pages[] = {
	VSCSI_ROM_PAGE(0x00, vTAPE_vpd_00),		/* Supported Pages */
	VSCSI_ROM_PAGE(0x80, vTAPE_vpd_80),		/* Unit Serial Number */
	VSCSI_ROM_PAGE(0x83, vTAPE_vpd_83),		/* Device Identification */
	VSCSI_ROM_PAGE(0,0)
};

/* Read-Write Error Recovery Mode Page */
static unsigned char vTAPE_mode_01[] = {
	0x01,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* Disconnect-Reconnect Mode Page */
static unsigned char vTAPE_mode_02[] = {
	0x02,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	/* 08 - 0f */
};

/* Control Mode Page */
static unsigned char vTAPE_mode_0a[] = {
	0x02,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,	/* 00 - 07 */
	0x00,0x00,0x00,0x00,                    	/* 08 - 0b */
};

/* vTAPE Supported Mode Page List */
static struct vscsi_rom_page vTAPE_mode_pages[] = {
	VSCSI_ROM_PAGE(0x01, vTAPE_mode_01),		/* Read-Write Error Recovery */
	VSCSI_ROM_PAGE(0x02, vTAPE_mode_02),		/* Disconnect-Reconnect */
	VSCSI_ROM_PAGE(0x0a, vTAPE_mode_0a),		/* Control */
	VSCSI_ROM_PAGE(0,0)
};

/* vTAPE ROM Definition */
static struct vscsi_rom vTAPE_rom = {
	4096,
	VSCSI_ROM_PAGE(0, vTAPE_std_page),
	vTAPE_vpd_pages,
	vTAPE_mode_pages
};


#if 0
#include <linux/crc32.h>
#ifdef __LITTLE_ENDIAN
#define crcfunc crc32_le
#else
#define crcfunc crc32_be
#endif
#endif

#define VSCSI_NUM_CHANS 2
#define VSCSI_NUM_IDS 4
#define VSCSI_NUM_LUNS 4

#ifdef current
#undef current
#endif

/* Device def */
struct vscsi_device {
	int chan,id,lun;			/* Self reference */
	int type;				/* SCSI device type */
	unsigned short flags;			/* Internal flags */
	struct vscsi_file *fp;			/* File ptr */
	struct vscsi_rom *rom;			/* ROM ptr */
	int key,asc,asq;			/* Sense info */
	void *priv;				/* Device specific info */
};

#define DEVICE_FLAG_INUSE	0x0001	/* Allocated */
#define DEVICE_FLAG_OPEN	0x0002	/* File open */
#define DEVICE_FLAG_RMB		0x0004	/* Removable */
#define DEVICE_FLAG_RO		0x0008	/* Read only */
#define DEVICE_FLAG_FORMAT	0x4000	/* Formatting */
#define DEVICE_FLAG_DEBUG	0x8000	/* Debugging */

#define DFLAG_ISSET(p,f) ( ( (p)->flags & DEVICE_FLAG_##f ) != 0 )
#define DFLAG_SET(p,f) (p)->flags |= DEVICE_FLAG_##f
#define DFLAG_UNSET(p,f) (p)->flags &= ~DEVICE_FLAG_##f

struct vscsi_id {
	struct vscsi_device luns[VSCSI_NUM_LUNS];
};

struct vscsi_channel {
	struct vscsi_id ids[VSCSI_NUM_IDS];
};

struct vscsi_channel channels[VSCSI_NUM_CHANS];
static struct vscsi_device dummy_device;

#define vscsi_device_clear(p) memset((p), 0, sizeof(struct vscsi_device)); (p)->type = 0x7F

#if 0
#define vscsi_get_device(c,i,l) &channels[c].ids[i].luns[l];
#else
static struct vscsi_device *vscsi_get_device(int r_chan, int r_id, int r_lun) {
	struct vscsi_device *dp;

	dprintk("vscsi_get_device: chan: %d(%d), id: %d(%d), lun: %d(%d)\n", 
		r_chan,VSCSI_NUM_CHANS,r_id,VSCSI_NUM_IDS,r_lun,VSCSI_NUM_LUNS);
	dp = 0;
	if (r_chan >= 0 && r_chan < VSCSI_NUM_CHANS) {
		struct vscsi_channel *chan;

		chan = &channels[r_chan];
		if (r_id >= 0 && r_id < VSCSI_NUM_IDS) {
			struct vscsi_id *id;

			id = &chan->ids[r_id];
			if (r_lun >= 0 && r_lun < VSCSI_NUM_LUNS)
				dp = &id->luns[r_lun];
		}
	}

	dprintk("vscsi_get_device: dp: %p\n", dp);
	if (!dp) dp = (struct vscsi_device *) &dummy_device;
	return dp;
}
#endif

static struct vscsi_device *_scan_luns(struct vscsi_id *ids, int r_lun) {
	struct vscsi_device *dp;
	int l;

	dp = 0;
	if (r_lun == -1) {
		for(l=0; l < VSCSI_NUM_LUNS; l++) {
			if (!DFLAG_ISSET(&ids->luns[l],INUSE)) {
				dp = &ids->luns[l];
				dp->lun = l;
				break;
			}
		}
	} else {
		if (!DFLAG_ISSET(&ids->luns[r_lun],INUSE)) {
			dp = &ids->luns[r_lun];
			dp->lun = r_lun;
		}
	}

	if (dp) DFLAG_SET(dp,INUSE);
	return dp;
}

static struct vscsi_device *_scan_ids(struct vscsi_channel *chan, int r_id, int r_lun) {
	struct vscsi_device *dp;
	int i;

	dp = 0;
	if (r_id == -1) {
		for(i=0; i < VSCSI_NUM_IDS; i++) {
			dp = _scan_luns(&chan->ids[i], r_lun);
			if (dp) {
				dp->id = i;
				break;
			}
		}
	} else {
		dp = _scan_luns(&chan->ids[r_id], r_lun);
		dp->id = r_id;
	}

	return dp;
}
		
static struct vscsi_device *vscsi_device_alloc(int r_chan, int r_id, int r_lun) {
	struct vscsi_device *dp;
	int c;

	dp = 0;
	if (r_chan == -1) {
		for(c=0; c < VSCSI_NUM_CHANS; c++) {
			dp = _scan_ids(&channels[c], r_id, r_lun);
			if (dp) {
				dp->chan = c;
				break;
			}
		}
	} else {
		dp = _scan_ids(&channels[r_chan], r_id, r_lun);
		dp->chan = r_chan;
	}

	return dp;
}

static void vscsi_device_free(struct vscsi_device *dp) {
	if (!dp) return;
	if (!DFLAG_ISSET(dp,INUSE)) return;

	/* Just clear it */
	vscsi_device_clear(dp);
}

static int vscsi_device_open(struct vscsi_device *dp) {
	dprintk("vscsi_device_open: open: %d\n", DFLAG_ISSET(dp,OPEN));
	if (DFLAG_ISSET(dp,OPEN)) return 0;

	/* Open the file */
	if (vscsi_file_open(dp->fp, DFLAG_ISSET(dp,RO)))
		return 1;

	DFLAG_SET(dp,OPEN);
	return 0;
}

static int vscsi_device_close(struct vscsi_device *dp) {

	dprintk("vscsi_device_close: open: %d\n", DFLAG_ISSET(dp,OPEN));
	if (!DFLAG_ISSET(dp,OPEN)) return 1;

	if (vscsi_file_close(dp->fp))
		return 1;

	DFLAG_UNSET(dp,OPEN);
	return 0;
}

static struct vscsi_device *vscsi_add_device(unsigned char type, unsigned char chan, char *path) {
	struct vscsi_device *dp;
//	unsigned long serial;

	dprintk("add_device: chan: %d, type: %d, path: %s\n", chan, type, path);

	dp = vscsi_device_alloc(-1,-1,-1);
	if (!dp) {
		dprintk(KERN_ERR "vSCSI: unable to alloc new device!\n");
		return 0;
	}
//	dprintk("vscsi_add_device: id: %d\n", dp->id);

	/* Alloc an fp for this device */
	dp->fp = vscsi_file_alloc(path,1);
	if (!dp->fp) {
		dprintk(KERN_ERR "vSCSI: unable to alloc new file!\n");
		vscsi_device_free(dp);
		return 0;
	}

	/* XXX */
	DFLAG_SET(dp,DEBUG);

	dp->type = type;
	switch(type) {
	case TYPE_DISK:
	case TYPE_MOD:
		dp->rom = &vDISK_rom;
		break;
	case TYPE_TAPE:
		dp->rom = &vTAPE_rom;
		break;
	case TYPE_WORM:
	case TYPE_ROM:
		dp->rom = &vCD_rom;
		dp->flags |= DEVICE_FLAG_RO;
		break;
	case TYPE_MEDIUM_CHANGER:
		dp->rom = &vCHANGER_rom;
		break;
	case TYPE_PASS:
		break;
	default:
		dprintk(KERN_ERR "vSCSI: no rom for type %d!\n", type);
		break;
	}

	if (dp->rom) {
		/* Set RMB flag from rom */
		if (dp->rom->std.data[1] & 0x80) dp->flags |= DEVICE_FLAG_RMB;

#if 0
		/* Generate serial from CRC of path */
		serial = crcfunc(~0, (unsigned char *) path, (size_t) strlen(path));
		if (DFLAG_ISSET(dp,DEBUG)) dprintk("vscsi_add_device: %lx\n", serial);

		/* Update the NAA locally-administrated value in the rom */
		if (dp->rom) {
			struct vscsi_rom_page *page;
			int i;
	
			i = 0;
			for(page = dp->rom->vpd; page; page++) {
				if (page->num == 0x83) {
					unsigned char *p = page->data;
					p[12] = (serial >> 24) & 0xff;
					p[13] = (serial >> 16) & 0xff;
					p[14] = (serial >> 8) & 0xff;
					p[15] = serial & 0xff;
					i = 1;
					break;
				}
			}
			if (!i) dprintk(KERN_WARNING "vSCSI: device_add: unable to update NAA value!\n");
		}
#endif
	}

	/* If not removable, try to access the file */
	if (!DFLAG_ISSET(dp, RMB)) {
		if (vscsi_device_open(dp)) {
			dprintk(KERN_ERR "vSCSI: unable to open %s, device not added.\n", path);
			vscsi_file_free(dp->fp);
			vscsi_device_free(dp);
			return 0;
		}
	}

	dprintk("vscsi_add_device: %d:%d:%d\n", dp->chan, dp->id, dp->lun);
	return dp;
}

static int vscsi_device_init(void) {
	struct vscsi_device *dp;
	int c,i,l;

	dprintk("vSCSI: initializing devices...\n");
	for(c=0; c < VSCSI_NUM_CHANS; c++) {
		for(i=0; i < VSCSI_NUM_IDS; i++) {
			for(l=0; l < VSCSI_NUM_LUNS; l++) {
				dp = vscsi_get_device(c,i,l);
				vscsi_device_clear(dp);
			}
		}
	}
	vscsi_device_clear(&dummy_device);
	return 0;
}


static int check_condition(struct vscsi_device *dp, int key, int asc, int asq) {
	dp->key = key;
	dp->asc = asc;
	dp->asq = asq;
	return CHECK_CONDITION;
}

#define VSCSI_DEBUG_SENSE 1

static int _add_page(int d, unsigned char *p, struct vscsi_rom_page *page) {
	memcpy(p, page->data, page->size);
	if (d) dump_data(scp->device->id, "page data", page->data, page->size);
	return page->size;
}

/* XXX buflen is the amount of bytes REMAINING in the buffer */
static int mode_sense(struct vscsi_device *dp, struct scsi_cmnd *scp, unsigned char *p, int buflen, int dl, int *data_len) {
	struct vscsi_rom_page *pages = dp->rom->mode;
	unsigned long long max_lba;
	unsigned char *oldp;
	int x,page;

//	calc_max_lba(dp->fp->size, scp->device->sector_size, &max_lba);
	calc_max_lba(dp->fp->size, dp->rom->sector_size, &max_lba);

	/* Save ptr */
	oldp = p;

	/* Generate block descriptor */
	if (dl == 8) {
		if (dp->type == 0) {
			/* Direct-access device mode parameter block descriptor */
			/* 0-3: NUMBER OF BLOCKS */
			if (max_lba > 0xfffffffe) {
			*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
			} else {
			*p++ = (max_lba >> 24) & 0xff;
				*p++ = (max_lba >> 16) & 0xff;
				*p++ = (max_lba >> 8) & 0xff;
				*p++ = max_lba & 0xff;
			}
		} else {
			/* General mode parameter block descriptor */
			/* 0: DENSITY CODE */
			*p++ = 0;
			/* 1-3: NUMBER OF BLOCKS */
			if (max_lba > 0xfffffe) {
			*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
			} else {
			*p++ = (max_lba >> 16) & 0xff;
				*p++ = (max_lba >> 8) & 0xff;
				*p++ = max_lba & 0xff;
			}
		}
		/* 4: DENSITY CODE/Reserved */
		*p++ = 0;
		/* 5-7: BLOCK LENGTH */
#if 0
		*p++ = (scp->device->sector_size >> 16) & 0xff;
		*p++ = (scp->device->sector_size >> 8) & 0xff;
		*p++ = scp->device->sector_size & 0xff;
#endif
		*p++ = (dp->rom->sector_size >> 16) & 0xff;
		*p++ = (dp->rom->sector_size >> 8) & 0xff;
		*p++ = dp->rom->sector_size & 0xff;
	} else if (dl == 16) {
		/* Long LBA mode parameter block descriptor */
		/* 0-7: NUMBER OF BLOCKS */
		*p++ = (max_lba >> 56) & 0xff;
		*p++ = (max_lba >> 48) & 0xff;
		*p++ = (max_lba >> 40) & 0xff;
		*p++ = (max_lba >> 32) & 0xff;
		*p++ = (max_lba >> 24) & 0xff;
		*p++ = (max_lba >> 16) & 0xff;
		*p++ = (max_lba >> 8) & 0xff;
		*p++ = max_lba & 0xff;
		/* 8: DENSITY CODE */
		*p++ = 0;
		/* 9-11: Reserved */
		*p++ = 0;
		*p++ = 0;
		*p++ = 0;
		/* 12-15: BLOCK LENGTH */
#if 0
		*p++ = (scp->device->sector_size >> 24) & 0xff;
		*p++ = (scp->device->sector_size >> 16) & 0xff;
		*p++ = (scp->device->sector_size >> 8) & 0xff;
		*p++ = scp->device->sector_size & 0xff;
#endif
		*p++ = (dp->rom->sector_size >> 24) & 0xff;
		*p++ = (dp->rom->sector_size >> 16) & 0xff;
		*p++ = (dp->rom->sector_size >> 8) & 0xff;
		*p++ = dp->rom->sector_size & 0xff;
	}

	buflen -= (p - oldp);

	page = scp->cmnd[2] & 0x3f;
#if VSCSI_DEBUG_SENSE
	if (DFLAG_ISSET(dp,DEBUG)) dprintk("page: %x\n", page);
#endif
	if (page == 0x3f) {
		/* All pages */
		if (scp->cmnd[3] == 0 || scp->cmnd[3] == 0xFF) {
			for(x=0; pages[x].data; x++) {
				if (pages[x].size < buflen) {
					p += _add_page(DFLAG_ISSET(dp,DEBUG),p,&pages[x]);
					buflen -= pages[x].size;
				}
			}
		} else
			return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	} else {
		/* Specific page */
		int found = 0;
		for(x=0; pages[x].data; x++) {
#if VSCSI_DEBUG_SENSE
			if (DFLAG_ISSET(dp,DEBUG)) dprintk("mode_sense: pages[%d].num: %x, page: %x\n", x, pages[x].num, page);
#endif
			if (pages[x].num == page) {
				p += _add_page(DFLAG_ISSET(dp,DEBUG),p,&pages[x]);
				found = 1;
				break;
			}
		}
		if (!found) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}

	*data_len = p - oldp;

	return GOOD;
}

static int read_write_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = (*(p+1) & 0x1f) << 16 | *(p+2) << 8 | *(p+3);
	num = *(p+4) ? *(p+4) : 0xff;

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}

static int read_write_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p  = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+7) << 8 | *(p+8);

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}

static int read_write_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}

static int read_write_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;
#if 0
	register int x;

	lba = 0;
	for (x = 0; x < 8; x++) {
		if (x) lba <<= 8;
		lba |= *(p+2+x);
	}
#endif
	lba =  ((u64)*(p+2)) << 56 | ((u64)*(p+3)) << 48 | ((u64)*(p+4)) << 40 | ((u64)*(p+5)) << 32 | \
		((u64)*(p+6)) << 24 | ((u64)*(p+7)) << 16 | ((u64)*(p+8)) << 8 | ((u64)*(p+9));
	num = *(p+10) << 24 | *(p+11) << 16 | *(p+12) << 8 | *(p+13);

	if (vscsi_file_rw(dp->fp, scp, lba, num, scp->cmnd[0] & 2))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}

static int vscsi_pass(struct vscsi_device *dp, struct scsi_cmnd *scp) {

	if (vscsi_file_pass(dp->fp, scp)) 
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	return 0;
}

/*
Status 		Sense Key 	Additional Sense Code
GOOD 		not applicable 	not applicable
CHECK CONDITION ILLEGAL REQUEST LOGICAL UNIT NOT SUPPORTED
CHECK CONDITION NOT READY 	LOGICAL UNIT DOES NOT RESPOND TO SELECTION
CHECK CONDITION NOT READY 	MEDIUM NOT PRESENT
CHECK CONDITION NOT READY	LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE
CHECK CONDITION NOT READY	LOGICAL UNIT IS IN PROCESS OF BECOMING READY
CHECK CONDITION NOT READY 	LOGICAL UNIT NOT READY, INITIALIZING COMMAND REQUIRED
CHECK CONDITION NOT READY 	LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED
CHECK CONDITION NOT READY	LOGICAL UNIT NOT READY, FORMAT IN PROGRESS

04h/00h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE
04h/01h  DTLPWROMAEBKVF  LOGICAL UNIT IS IN PROCESS OF BECOMING READY
04h/02h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, INITIALIZING COMMAND REQUIRED
04h/03h  DTLPWROMAEBKVF  LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED
04h/04h  DTL  RO   B     LOGICAL UNIT NOT READY, FORMAT IN PROGRESS

*/

#define CAUSE_NOT_REPORTABLE 0x00
#define INITIALIZING_COMMAND_REQUIRED 0x02
#define FORMAT_IN_PROGRESS 0x04

static int test_unit_ready(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rc;

	dprintk("test_unit_ready: open: %d\n", DFLAG_ISSET(dp, OPEN));
	if (DFLAG_ISSET(dp, OPEN)) {
		dprintk("test_unit_ready: format: %d\n", DFLAG_ISSET(dp, FORMAT));
		if (DFLAG_ISSET(dp, FORMAT))
			rc = check_condition(dp, NOT_READY, LOGICAL_UNIT_NOT_READY, FORMAT_IN_PROGRESS);
		else
			rc = GOOD;
	} else {
		dprintk("test_unit_ready: removable: %d\n", DFLAG_ISSET(dp, RMB));
		if (DFLAG_ISSET(dp, RMB)) {
//			rc = check_condition(dp, NOT_READY, MEDIUM_NOT_PRESENT, 0);
			rc = GOOD;
		}
		else
			rc = check_condition(dp, NOT_READY, LOGICAL_UNIT_NOT_READY, CAUSE_NOT_REPORTABLE);
	}

	return rc;
}

#if 0
static int unit_ready(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int error, rc;

	rc = GOOD;
	error = (wp->dp->os_handle == 0 ? host_open(wp->dp) : GOOD);
	if (error) {
		switch(wp->dp->type) {
		case TYPE_ROM:
		case TYPE_TAPE:
			rc = check_condition(wp->dp, NOT_READY, MEDIUM_NOT_PRESENT, 0x2);
			break;
		default:
			rc = check_condition(wp->dp, NOT_READY, LOGICAL_UNIT_NOT_READY, 0x2);
			break;
		}
	}

	return rc;
}
#endif
static int rewind(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int request_sense(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	memset(temp, 0, 18);
	if (scp->cmnd[1] & 1) {
		temp[0] = 0x72;
		temp[1] = dp->key;
		temp[2] = dp->asc;
		temp[3] = dp->asq;
	} else {
		temp[0] = 0x70;
		temp[2] = dp->key;
		temp[7] = 0xa;
		temp[12] = dp->asc;
		temp[13] = dp->asq;
	}
	return response(scp, &temp, min(scp->cmnd[4], 18));
}

static int format_unit(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_block_limits(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int reassign_blocks(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


#define read_6 read_write_6
#if 0
static int read_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = (*(p+1) & 0x1f) << 16 | *(p+2) << 8 | *(p+3);
	num = *(p+4) ? *(p+4) : 0xff;

	if (vscsi_file_read(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

#define write_6 read_write_6
#if 0
static int write_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = (*(p+1) & 0x1f) << 16 | *(p+2) << 8 | *(p+3);
	num = *(p+4) ? *(p+4) : 0xff;

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif
static int seek_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_reverse_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_filemarks_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int space_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


#define DEBUG_INQ 0

static int inquiry(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int x, alloc_len;

	alloc_len = (scp->cmnd[3] << 8) + scp->cmnd[4];
#if DEBUG_INQ
	if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: alloc_len: %d\n", alloc_len);
#endif

	/* EVPD? */
	if (scp->cmnd[1] & 1) {
		struct vscsi_rom_page *vpd = dp->rom->vpd;
		int page = scp->cmnd[2];

#if DEBUG_INQ
		if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: sending VPD page %d\n", page);
#endif
		for(x=0; vpd[x].data; x++) {
			if (vpd[x].num == page)
				return response(scp, vpd[x].data, min(alloc_len, vpd[x].size));
		}
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	/* Standard page */
	} else {
		/* If the EVPD bit is set to zero, the device server shall return the standard INQUIRY
		   data (see 6.4.2). If the PAGE CODE field is not set to zero when the EVPD bit is set
		   to zero, the command shall be terminated with CHECK CONDITION status, with the sense
		   key set to ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN CDB. */
		if (scp->cmnd[2] != 0) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

#if DEBUG_INQ
		if (DFLAG_ISSET(dp,DEBUG)) dprintk(KERN_INFO "scsi_inq: sending STD page\n");
#endif
#if 0
		{
			unsigned char *data;

			data = dp->rom->std.data;
			printk("INQUIRY: ADDR16=%d\n", (data[6] & 0x01) != 0);
			printk("INQUIRY: WBUS16=%d\n", (data[7] & 0x20) != 0);
			printk("INQUIRY: SYNC=%d\n", (data[7] & 0x10) != 0);
			printk("INQUIRY: CMDQUE=%d\n", (data[7] & 0x02) != 0);
		}
#endif

		return response(scp, dp->rom->std.data, min(alloc_len, dp->rom->std.size));
	}
}
static int verify_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int recover_buffered_data(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int mode_select_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int reserve_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int release_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int copy(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int erase_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


#define VSCSI_DEBUG_SENSE 1

static int mode_sense_6(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int err,dl,buflen,data_len;

	/* DBD */
	dl = (scp->cmnd[1] & 0x08 ? 0 : 8);
        buflen = min(scp->cmnd[4],sizeof(temp));

	/* MEDIUM TYPE */
	temp[1] = 0;
	/* DEVICE-SPECIFIC PARAMETER */
	temp[2] = 0;
	/* BLOCK DESCRIPTOR LENGTH */
	temp[3] = dl;

	err =  mode_sense(dp, scp, &temp[4], buflen - 4, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* MODE DATA LENGTH */
	temp[0] = data_len;
	return response(scp, &temp, data_len);
}

static int start_stop_unit(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int start;

#if 0
0 OPERATION CODE (1Bh)
1 Reserved IMMED
2 Reserved
3 Reserved POWER CONDITION MODIFIER
4 POWER CONDITION Reserved NO_FLUSH LOEJ START
5 CONTROL
#endif
#if 0
POWER CONDITION field
0h START_VALID Process the START and LOEJ bits.
1h ACTIVE Place the device into the active power condition.
2h IDLE Place the device into the idle power condition.
3h STANDBY Place the device into the standby power condition.
4h Reserved Reserved
5h Obsolete Obsolete
6h Reserved Reserved
7h LU_CONTROL Transfer control of power conditions to the logical unit.
8h to 9h Reserved Reserved
Ah FORCE_IDLE_0 Force the idle condition timer to zero.
Bh FORCE_STANDBY_0 Force the standby condition timer to zero.
Ch to Fh Reserved Reserved
#endif
	start = scp->cmnd[4] & 1;
	dprintk("start: %d\n", start);
//	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	return GOOD;
}

static int receive_diagnostic_results(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int send_diagnostic(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int prevent_allow_medium_removal(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rc,prevent;

	prevent = scp->cmnd[4] & 1;
	dprintk("prevent: %d\n", prevent);
	rc = GOOD;

	/* Removable? */
	if (DFLAG_ISSET(dp, RMB)) {
		if (prevent) {
			/* Already open? */
			if (DFLAG_ISSET(dp,OPEN)) {
//				dp->flags |= DEVICE_FLAG_LOCKED;
				rc = 0;
			} else {
				/* Open it */
				if (vscsi_device_open(dp))
					rc = check_condition(dp, NOT_READY, MEDIUM_NOT_PRESENT, 0);
				else
					rc = 0;
			}
		} else {
			if (DFLAG_ISSET(dp,OPEN)) vscsi_device_close(dp);
			rc = 0; /* XXX */
		}
	} else
		rc = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	return rc;
}

static int read_format_capacities(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_capacity_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long max_lba;
#if VSCSI_STRICT
	unsigned long long lba;
	unsigned char *p = scp->cmnd;

	/* Get the lba requested */
	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);

	/* The LOGICAL BLOCK ADDRESS field shall be set to zero if the PMI bit is set to zero. If the PMI bit is set to zero and
the LOGICAL BLOCK ADDRESS field is not set to zero, then the device server shall terminate the command with
CHECK CONDITION status with the sense key set to ILLEGAL REQUEST and the additional sense code set
to INVALID FIELD IN CDB */
	if ((p[8] & 1) == 0 && lba != 0) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif

//	calc_max_lba(dp->fp->size, scp->device->sector_size, &max_lba);
	calc_max_lba(dp->fp->size, dp->rom->sector_size, &max_lba);
	if (max_lba > 0xfffffffe || scp->cmnd[8] & 1) {
		temp[0] = 0xff;
		temp[1] = 0xff;
		temp[2] = 0xff;
		temp[3] = 0xff;
	} else {
		temp[0] = (max_lba >> 24);
		temp[1] = (max_lba >> 16) & 0xff;
		temp[2] = (max_lba >> 8) & 0xff;
		temp[3] = max_lba & 0xff;
	}
#if 0
	temp[4] = (scp->device->sector_size >> 24);
	temp[5] = (scp->device->sector_size >> 16) & 0xff;
	temp[6] = (scp->device->sector_size >> 8) & 0xff;
	temp[7] = scp->device->sector_size & 0xff;
#endif
	temp[4] = (dp->rom->sector_size >> 24);
	temp[5] = (dp->rom->sector_size >> 16) & 0xff;
	temp[6] = (dp->rom->sector_size >> 8) & 0xff;
	temp[7] = dp->rom->sector_size & 0xff;

	return response(scp, &temp, 8);
}
#define read_10 read_write_10
#if 0
static int read_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p  = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+7) << 8 | *(p+8);
	if (vscsi_file_read(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int read_generation(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

#define write_10 read_write_10
#if 0
static int write_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p  = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+7) << 8 | *(p+8);

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int seek_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int erase_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_updated_block(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_and_verify_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int verify_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int set_limits_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int pre_fetch_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int synchronize_cache_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int lock_unlock_cache_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_defect_data_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int medium_scan(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int compare(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int copy_and_verify(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_buffer(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int mode, id, off, len;
#if 0
0 OPERATION CODE (3Bh)
1 Reserved MODE
2 BUFFER ID
3 (MSB) BUFFER OFFSET
5 (LSB)
6 (MSB) PARAMETER LIST LENGTH
8 (LSB)
9 CONTROL
#endif
	mode = scp->cmnd[1] & 0xF;
	id = scp->cmnd[2];
	off = scp->cmnd[3] << 16 | scp->cmnd[4] << 8 | scp->cmnd[5];
	len = scp->cmnd[6] << 16 | scp->cmnd[7] << 8 | scp->cmnd[8];
	dprintk("write_buffer: mode: %x, id: %d, off: %x, len: %d\n", mode, id, off, len);

	if (mode == 0x0A) {
		if (off + len > sizeof(echobuf))
			return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
		return 0;
	} else {
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}
}

static int read_buffer(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int mode, id, off, len;

#if 0
0 OPERATION CODE (3Ch)
1 Reserved MODE
2 BUFFER ID
3 (MSB) BUFFER OFFSET
5 (LSB)
6 (MSB) ALLOCATION LENGTH
8 (LSB)
9 CONTROL
#endif
	mode = scp->cmnd[1] & 0x0F;
	id = scp->cmnd[2];
	off = scp->cmnd[3] << 16 | scp->cmnd[4] << 8 | scp->cmnd[5];
	len = scp->cmnd[6] << 16 | scp->cmnd[7] << 8 | scp->cmnd[8];
	dprintk("read_buffer: mode: %x, id: %d, off: %d, len: %d\n", mode, id, off, len);

	/* Echo buffer descriptor mode */
	if (mode == 0x0B && len == 4) {
		temp[0] = 0;
		temp[1] = 0;
		temp[2] = 0x10;
		temp[4] = 0;
		return response(scp, &temp, 4);
	} else {
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}
}

static int update_block(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_long_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_long_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int change_definition(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_same_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_sub_channel(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_toc_pma_atip(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int time,format,track,alloc_len;
	unsigned char *p;

#if 0
0 OPERATION CODE (43h)
1 Reserved Reserved TIME Reserved
2 Reserved Format
3 Reserved
4 Reserved
5 Reserved
6 Track/Session Number
7 (MSB) Allocation Length
8 (LSB)
9 Control
#endif

	time = ((scp->cmnd[0] & 2) != 0);
	format = scp->cmnd[2] & 0x0f;
	track = scp->cmnd[6];
	alloc_len = (scp->cmnd[7] << 8) | scp->cmnd[8];
	p = &temp[2];

	dprintk("time: %d, format: %x, track: %d, alloc_len: %d\n", time, format, track, alloc_len);

	switch(format) {
	case 0:
		/* The Track/Session Number field specifies starting track number for which the data is returned. For multi-session discs, this command returns the TOC data for all sessions and for Track number AAh only the Lead-out area of the last complete session. */
#if 0
0 (MSB) TOC Data Length
1 (LSB)
2 First Track Number(Hex)
3 Last Track Number(Hex)
TOC Track Descriptor(s)
0 Reserved
1 ADR CONTROL
2 Track Number(Hex)
3 Reserved
4 (MSB)
… Track Start Address
7 (LSB)
#endif
		/* Header */
		*p++ = 1;		/* First track # */
		*p++ = 1;		/* Last track # */

		/* Track Descriptor for track 0 */
		*p++ = 0;		/* Reserved */
		*p++ = 6;		/* ADR & Control (Data,uninterupted+copy permitted */
		*p++ = 1;		/* Track # */
		*p++ = 0;		/* Reserved */
		*p++ = 0;		/* Track start MSB */
		*p++ = 0;
		*p++ = 0;
		*p++ = 16;		/* Track start LSB */
		temp[1] = p - temp;
		break;
	case 1:
		/* This format returns the first complete session number, last complete session number and last complete session starting address. In this format, the Track/Session Number field is reserved and should be set to 00h. NOTE: This format provides the Initiator access to the last finalized session starting address quickly. */
		break;
	case 2:
		/* This format returns all Q sub-code data in the Lead-In (TOC) areas starting from a session number as specified in the Track/Session Number field. In this mode, the Logical Unit shall support Q Sub-channel POINT field value of A0h, A1h, A2h, Track numbers, B0h, B1h, B2h, B3h, B4h, C0h, and C1h. See Table 236. There is no defined LBA addressing and TIME bit shall be set to one. */
	case 3:
		/* This format returns all Q sub-code data in the PMA area. In this format, the Track/Session Number field is reserved and shall be set to 00h. See Table 241. There is no defined LBA addressing and TIME bit shall be set to one. */
	case 4:
		/* This format returns ATIP data. In this format, the Track/Session Number field is reserved and shall be set to 00h. See Table 242. There is no defined LBA addressing and TIME bit shall be set to one */
	case 5:
		/* This format returns CD-TEXT information that is recorded in the Lead-in area as R-W Sub-channel Data */
	default:
		return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
	}

	return response(scp, temp, min(alloc_len, temp[1]));
}

static int report_density_support(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int play_audio_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


#if 0
static unsigned short dvd_features[] = {
	0x00,		/* List */
	0x01,		/* Core */
	0x02, 		/* Morphing */
	0x03,		/* Removable */
	0x04,		/* Write Protect */
	0x10,		/* Random Readable */
#if 0
	0x1D,		/* Multi-Read */
#endif
	0x1E,		/* CD */
	0x1F,		/* DVD */
};

static unsigned short supported_profiles[] = {
	0x08,		/* CD-ROM */
	0x10,		/* DVD-ROM */
};
#endif

static int get_configuration(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int rt, sfn, alloc_len;
	unsigned char *p = temp;

#if 0
0 OPERATION CODE (46h)
1 Reserved Reserved RT
2 (MSB) Starting Feature Number
3 (LSB)
4 Reserved
5 Reserved
6 Reserved
7
8
(MSB) Allocation Length
(LSB)
9 Control
#endif
	rt = scp->cmnd[1] & 2;
	sfn = (scp->cmnd[2] << 8) | scp->cmnd[3];
	alloc_len = (scp->cmnd[7] << 8) | scp->cmnd[8];

	dprintk("rt: %d, sfn: %d, alloc_len: %d\n", rt, sfn, alloc_len);

	/* An Allocation Length field of zero indicates that no data shall be transferred. This condition shall not be
considered an error. */
	if (alloc_len == 0) return GOOD;

	/* RT 0=all, 1=all current, 2=single */
	memset(temp,0,min(alloc_len,sizeof(temp)));

	if (!DFLAG_ISSET(dp,OPEN) || dp->type != TYPE_ROM) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	switch(rt) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	}

	temp[7] = 0x10;		/* DVD Profile */
	p += 8;

#if 0
Mandatory Features for DVD-ROM
0000h Profile List A list of all Profiles supported by the device
0001h Core Basic Functionality
0002h Morphing The device changes its operational behavior due to external events
0003h Removable Medium The medium may be removed from the device
0010h Random Readable, PP=1 Read ability for storage devices with random addressing.
001Fh DVD Read The ability to read DVD specific structures
0100h Power Management Initiator and device directed power management
0105h Timeout Ability to respond to all commands within a specific time
0107h Real-Time Streaming Ability to read using Initiator requested performance parameters
#endif
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int play_audio_msf(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int get_event_status_notification(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int pause_resume(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int log_select(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int log_sense(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int stop_play_scan(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int xdwrite_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int xpwrite_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int xdread_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int reserve_track(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int send_opc_information(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int mode_select_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int reserve_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int release_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int repair_track(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}


#define VSCSI_DEBUG_SENSE 1

static int mode_sense_10(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	int err,dl,alloc_len,buflen,data_len;

	/* DBD */
	if (scp->cmnd[1] & 0x08) {
		dl = 0;
	} else {
		/* LLBA */
		if (scp->cmnd[1] & 0x10)
			dl = 16;
		else
			dl = 8;
	}

	alloc_len = scp->cmnd[7] << 8 | scp->cmnd[8];
	buflen = min(alloc_len,sizeof(temp));

	/* 2 MEDIUM TYPE */
	temp[2] = 0;
	/* 3 DEVICE-SPECIFIC PARAMETER */
	temp[3] = 0;
	/* 4 Reserved LONGLBA */
	temp[4] = 1;
	/* 5 Reserved */
	temp[5] = 0;
	/* 6-7 BLOCK DESCRIPTOR LENGTH */
	temp[6] = 0;
	temp[7] = dl;

	err =  mode_sense(dp, scp, &temp[8], buflen - 8, dl, &data_len);
	if (err) return err;

	dprintk("data_len: %d\n", data_len);

	/* 0-1 MODE DATA LENGTH */
	temp[0] = (data_len >> 8) & 0xff;
	temp[1] = data_len & 0xff;
	return response(scp, &temp, data_len);
}

static int close_track_session(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_buffer_capacity(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int send_cue_sheet(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int persistent_reserve_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int persistent_reserve_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int extended_cdb(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int variable_length_cdb(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int extended_copy(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int receive_copy_results(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int ata_command_pass_through_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int access_control_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int access_control_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

#define read_16 read_write_16
#if 0
static int read_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;
	register int x;

	lba = 0;
	for (x = 0; x < 8; x++) {
		if (x) lba <<= 8;
		lba |= *(p+2+x);
	}
	num = *(p+10) << 24 | *(p+11) << 16 | *(p+12) << 8 | *(p+13);

	if (vscsi_file_read(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

#define write_16 read_write_16
#if 0
static int write_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;
	register int x;

	lba = 0;
	for (x = 0; x < 8; x++) {
		if (x) lba <<= 8;
		lba |= *(p+2+x);
	}
	num = *(p+10) << 24 | *(p+11) << 16 | *(p+12) << 8 | *(p+13);

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int orwrite(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_attribute(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_attribute(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_and_verify_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int verify_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int pre_fetch_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int synchronize_cache_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int lock_unlock_cache_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_same_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int service_action_out_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int report_luns(struct vscsi_device *dp, struct scsi_cmnd *scp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	unsigned char *p = scp->cmnd, *lp;
	int len;
	int i,t,count;
	struct vscsi_device *ldp;
	struct scsi_lun lun;

	/* NOTE 38 - Device servers compliant with SPC return CHECK CONDITION status, with the sense key set to
	   ILLEGAL REQUEST, and the additional sense code set to INVALID FIELD IN CDB when the allocation length is
	   less than 16 bytes. */
	len = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);
	if (len < 16) return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

	/* Get a count of the luns */
	count = 0;
	for(i=0; i < VSCSI_NUM_LUNS; i++) {
		ldp = vscsi_get_device(dp->chan, dp->id, i);
		if (!DFLAG_ISSET(ldp,INUSE)) break;
		count++;
	}
	dprintk("report_luns: count: %d\n", count);

	/*  If the device server is not ready with the logical unit inventory or if the inventory list is null for the requesting I_T nexus and the SELECT REPORT field set to 02h, then the device server shall provide a default logical unit inventory that contains at least LUN 0 or the REPORT LUNS well known logical unit (see 8.2). A non-empty peripheral device logical unit inventory that does not contain either LUN 0 or the REPORT LUNS well known logical unit is valid. */
	memset(temp, 0, 16);
	t = 16;
	switch(*(p+2)) {
	case 0: /* Addressing */
	case 2: /* All - only 1 lun (0) supported */
		len = count * 8;
		temp[0] = (len >> 24) & 0xFF;
		temp[1] = (len >> 16) & 0xFF;
		temp[2] = (len >> 8) & 0xFF;
		temp[3] = len & 0xFF;
		t = 8;
		for(i=0; i < count; i++) {
			int_to_scsilun(i, &lun);
			lp = (unsigned char *) &lun;
			temp[t++] = lp[0];
			temp[t++] = lp[1];
			temp[t++] = lp[2];
			temp[t++] = lp[3];
			temp[t++] = lp[4];
			temp[t++] = lp[5];
			temp[t++] = lp[6];
			temp[t++] = lp[7];
		}
		break;
	case 1: /* Well-known (not supported) */
	default:
		t = 16;
		break;
	}
	dprintk("report_luns: t: %d\n", t);
	scp->result = response(scp, &temp, t);

	return 0;
#else
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif
}

static int blank(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int security_protocol_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int maintenance_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int maintenance_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int move_medium(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int exchange_medium(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int move_medium_attached(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

#define read_12 read_write_12
#if 0
static int read_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);

	if (vscsi_file_read(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int service_action_out_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

#define write_12 read_write_12
#if 0
static int write_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	unsigned long long lba;
	unsigned long num;
	register unsigned char *p = scp->cmnd;

	lba = *(p+2) << 24 | *(p+3) << 16 | *(p+4) << 8 | *(p+5);
	num = *(p+6) << 24 | *(p+7) << 16 | *(p+8) << 8 | *(p+9);

	if (vscsi_file_write(dp->fp, scp, lba, num))
		return check_condition(dp, HARDWARE_ERROR, 0x3e, 1);
	else
		return GOOD;
}
#endif

static int service_action_in_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int erase_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_dvd_structure(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_and_verify_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int verify_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int set_limits_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_element_status_attached(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int security_protocol_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int send_volume_tag(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_defect_data_12(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_element_status(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_cd_msf(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int redundancy_group_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int redundancy_group_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int spare_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int spare_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int volume_set_in(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int volume_set_out(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int write_filemarks_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

static int read_reverse_16(struct vscsi_device *dp, struct scsi_cmnd *scp) {
	return check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
}

#define VSCSI_NTYPES 20
#if VSCSI_STRICT
static unsigned char scsi_commands[VSCSI_NTYPES][32] = {
	{ /* 00 - Disk */
		0x99, 0x05, 0x24, 0x7C, 0x20, 0xC5, 0xB0, 0xD8, 
		0x02, 0x30, 0x27, 0xC4, 0x00, 0x00, 0x00, 0xC0, 
		0xF8, 0xFD, 0x0B, 0x00, 0x1F, 0xC5, 0xA0, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 01 - Tape */
		0x3B, 0x8D, 0x3F, 0x7E, 0x00, 0x08, 0x10, 0x18, 
		0x10, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xDB, 0xB5, 0x0E, 0x00, 0x3D, 0x00, 0x20, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 02 - Printer */
		0x19, 0x0C, 0xF5, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 03 - Processor */
		0x09, 0x05, 0x04, 0x30, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 04 - WORM */
		0x89, 0x05, 0xE4, 0x7D, 0x20, 0xCD, 0x78, 0xDF, 
		0x01, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0xF5, 0x07, 0x00, 0xB9, 0xC5, 0x18, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 05 - CD/DVD */
		0x19, 0x08, 0x04, 0x48, 0x28, 0xDD, 0x20, 0x18, 
		0xEC, 0x4C, 0x3E, 0x3D, 0x00, 0x00, 0x00, 0x40, 
		0x20, 0x00, 0x00, 0x00, 0xFE, 0x3D, 0x60, 0xEE, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 06 - Scanner */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 07 - Optical */
		0x99, 0x05, 0xE4, 0x7D, 0x20, 0xFF, 0xF8, 0xFF, 
		0x01, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0xF5, 0x07, 0x00, 0xB9, 0xD5, 0x98, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 08 - Changer */
		0x89, 0x00, 0xE4, 0x7C, 0x00, 0x08, 0x80, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x30, 0x00, 0x00, 0x79, 0x00, 0x60, 0xFD, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 09 - Communication */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 10 - Obsolete */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 11 - Obsolete */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 12 - Raid */
		0x09, 0x00, 0xE4, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0xE0, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 13 - Enclosure */
		0x09, 0x00, 0x24, 0x34, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x40, 
		0xC0, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0xFC, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 14 - Simple Disk */
		0x09, 0x00, 0x04, 0x08, 0x20, 0x45, 0x20, 0x08, 
		0x00, 0x00, 0x20, 0x04, 0x00, 0x00, 0x00, 0x40, 
		0xE0, 0xF5, 0x03, 0x00, 0x1A, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 15 - Card Reader */
		0x09, 0x00, 0xE4, 0x7C, 0x20, 0x4D, 0x70, 0x19, 
		0x00, 0x30, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00, 
		0xD8, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 16 - Bridge */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 17 - OSD */
		0x09, 0x00, 0x04, 0x78, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x80, 
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 18 - Automation */
		0x09, 0x00, 0x24, 0x3C, 0x00, 0x00, 0x00, 0x18, 
		0x10, 0x30, 0x20, 0x04, 0x00, 0x00, 0x00, 0x40, 
		0xD8, 0x30, 0x00, 0x80, 0x1D, 0x0A, 0x20, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
	{ /* 19 - Security */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	},
};
#endif

typedef int (*vscsi_opfunc_t)(struct vscsi_device *, struct scsi_cmnd *);

#define VSCSI_MAX_OP 0xbf
struct vscsi_op {
	char *name;
	vscsi_opfunc_t func;
} vscsi_ops[VSCSI_MAX_OP+1] = {
	{ "test_unit_ready",test_unit_ready },						/* 00 */
	{ "rewind",rewind },							/* 01 */
	{ 0,0 },								/* 02 */
	{ "request_sense",request_sense },						/* 03 */
	{ "format_unit",format_unit },						/* 04 */
	{ "read_block_limits",read_block_limits },						/* 05 */
	{ 0,0 },								/* 06 */
	{ "reassign_blocks",reassign_blocks },						/* 07 */
	{ "read_6",read_6 },							/* 08 */
	{ 0,0 },								/* 09 */
	{ "write_6",write_6 },							/* 0a */
	{ "seek_6",seek_6 },							/* 0b */
	{ 0,0 },								/* 0c */
	{ 0,0 },								/* 0d */
	{ 0,0 },								/* 0e */
	{ "read_reverse_6",read_reverse_6 },						/* 0f */
	{ "write_filemarks_6",write_filemarks_6 },						/* 10 */
	{ "space_6",space_6 },							/* 11 */
	{ "inquiry",inquiry },							/* 12 */
	{ "verify_6",verify_6 },							/* 13 */
	{ "recover_buffered_data",recover_buffered_data },					/* 14 */
	{ "mode_select_6",mode_select_6 },						/* 15 */
	{ "reserve_6",reserve_6 },							/* 16 */
	{ "release_6",release_6 },							/* 17 */
	{ "copy",copy },							/* 18 */
	{ "erase_6",erase_6 },							/* 19 */
	{ "mode_sense_6",mode_sense_6 },						/* 1a */
	{ "start_stop_unit",start_stop_unit },						/* 1b */
	{ "receive_diagnostic_results",receive_diagnostic_results },					/* 1c */
	{ "send_diagnostic",send_diagnostic },						/* 1d */
	{ "prevent_allow_medium_removal",prevent_allow_medium_removal },				/* 1e */
	{ 0,0 },								/* 1f */
	{ 0,0 },								/* 20 */
	{ 0,0 },								/* 21 */
	{ 0,0 },								/* 22 */
	{ "read_format_capacities",read_format_capacities },					/* 23 */
	{ 0,0 },								/* 24 */
	{ "read_capacity_10",read_capacity_10 },						/* 25 */
	{ 0,0 },								/* 26 */
	{ 0,0 },								/* 27 */
	{ "read_10",read_10 },							/* 28 */
	{ "read_generation",read_generation },						/* 29 */
	{ "write_10",write_10 },							/* 2a */
	{ "seek_10",seek_10 },							/* 2b */
	{ "erase_10",erase_10 },							/* 2c */
	{ "read_updated_block",read_updated_block },						/* 2d */
	{ "write_and_verify_10",write_and_verify_10 },					/* 2e */
	{ "verify_10",verify_10 },							/* 2f */
	{ 0,0 },								/* 30 */
	{ 0,0 },								/* 31 */
	{ 0,0 },								/* 32 */
	{ "set_limits_10",set_limits_10 },						/* 33 */
	{ "pre_fetch_10",pre_fetch_10 },						/* 34 */
	{ "synchronize_cache_10",synchronize_cache_10 },					/* 35 */
	{ "lock_unlock_cache_10",lock_unlock_cache_10 },					/* 36 */
	{ "read_defect_data_10",read_defect_data_10 },					/* 37 */
	{ "medium_scan",medium_scan },						/* 38 */
	{ "compare",compare },							/* 39 */
	{ "copy_and_verify",copy_and_verify },						/* 3a */
	{ "write_buffer",write_buffer },						/* 3b */
	{ "read_buffer",read_buffer },						/* 3c */
	{ "update_block",update_block },						/* 3d */
	{ "read_long_10",read_long_10 },						/* 3e */
	{ "write_long_10",write_long_10 },						/* 3f */
	{ "change_definition",change_definition },						/* 40 */
	{ "write_same_10",write_same_10 },						/* 41 */
	{ "read_sub_channel",read_sub_channel },						/* 42 */
	{ "read_toc_pma_atip",read_toc_pma_atip },						/* 43 */
	{ "report_density_support",report_density_support },					/* 44 */
	{ "play_audio_10",play_audio_10 },						/* 45 */
	{ "get_configuration",get_configuration },						/* 46 */
	{ "play_audio_msf",play_audio_msf },						/* 47 */
	{ 0,0 },								/* 48 */
	{ 0,0 },								/* 49 */
	{ "get_event_status_notification",get_event_status_notification },				/* 4a */
	{ "pause_resume",pause_resume },						/* 4b */
	{ "log_select",log_select },							/* 4c */
	{ "log_sense",log_sense },							/* 4d */
	{ "stop_play_scan",stop_play_scan },						/* 4e */
	{ 0,0 },								/* 4f */
	{ "xdwrite_10",xdwrite_10 },							/* 50 */
	{ "xpwrite_10",xpwrite_10 },							/* 51 */
	{ "xdread_10",xdread_10 },							/* 52 */
	{ "reserve_track",reserve_track },						/* 53 */
	{ "send_opc_information",send_opc_information },					/* 54 */
	{ "mode_select_10",mode_select_10 },						/* 55 */
	{ "reserve_10",reserve_10 },							/* 56 */
	{ "release_10",release_10 },							/* 57 */
	{ "repair_track",repair_track },						/* 58 */
	{ 0,0 },								/* 59 */
	{ "mode_sense_10",mode_sense_10 },						/* 5a */
	{ "close_track_session",close_track_session },					/* 5b */
	{ "read_buffer_capacity",read_buffer_capacity },					/* 5c */
	{ "send_cue_sheet",send_cue_sheet },						/* 5d */
	{ "persistent_reserve_in",persistent_reserve_in },					/* 5e */
	{ "persistent_reserve_out",persistent_reserve_out },					/* 5f */
	{ 0,0 },								/* 60 */
	{ 0,0 },								/* 61 */
	{ 0,0 },								/* 62 */
	{ 0,0 },								/* 63 */
	{ 0,0 },								/* 64 */
	{ 0,0 },								/* 65 */
	{ 0,0 },								/* 66 */
	{ 0,0 },								/* 67 */
	{ 0,0 },								/* 68 */
	{ 0,0 },								/* 69 */
	{ 0,0 },								/* 6a */
	{ 0,0 },								/* 6b */
	{ 0,0 },								/* 6c */
	{ 0,0 },								/* 6d */
	{ 0,0 },								/* 6e */
	{ 0,0 },								/* 6f */
	{ 0,0 },								/* 70 */
	{ 0,0 },								/* 71 */
	{ 0,0 },								/* 72 */
	{ 0,0 },								/* 73 */
	{ 0,0 },								/* 74 */
	{ 0,0 },								/* 75 */
	{ 0,0 },								/* 76 */
	{ 0,0 },								/* 77 */
	{ 0,0 },								/* 78 */
	{ 0,0 },								/* 79 */
	{ 0,0 },								/* 7a */
	{ 0,0 },								/* 7b */
	{ 0,0 },								/* 7c */
	{ 0,0 },								/* 7d */
	{ "extended_cdb",extended_cdb },						/* 7e */
	{ "variable_length_cdb",variable_length_cdb },					/* 7f */
	{ "write_filemarks_16",write_filemarks_16 },						/* 80 */
	{ "read_reverse_16",read_reverse_16 },						/* 81 */
	{ 0,0 },								/* 82 */
	{ "extended_copy",extended_copy },						/* 83 */
	{ "receive_copy_results",receive_copy_results },					/* 84 */
	{ "ata_command_pass_through_16",ata_command_pass_through_16 },				/* 85 */
	{ "access_control_in",access_control_in },						/* 86 */
	{ "access_control_out",access_control_out },						/* 87 */
	{ "read_16",read_16 },							/* 88 */
	{ 0,0 },								/* 89 */
	{ "write_16",write_16 },							/* 8a */
	{ "orwrite",orwrite },							/* 8b */
	{ "read_attribute",read_attribute },						/* 8c */
	{ "write_attribute",write_attribute },						/* 8d */
	{ "write_and_verify_16",write_and_verify_16 },					/* 8e */
	{ "verify_16",verify_16 },							/* 8f */
	{ "pre_fetch_16",pre_fetch_16 },						/* 90 */
	{ "synchronize_cache_16",synchronize_cache_16 },					/* 91 */
	{ "lock_unlock_cache_16",lock_unlock_cache_16 },					/* 92 */
	{ "write_same_16",write_same_16 },						/* 93 */
	{ 0,0 },								/* 94 */
	{ 0,0 },								/* 95 */
	{ 0,0 },								/* 96 */
	{ 0,0 },								/* 97 */
	{ 0,0 },								/* 98 */
	{ 0,0 },								/* 99 */
	{ 0,0 },								/* 9a */
	{ 0,0 },								/* 9b */
	{ 0,0 },								/* 9c */
	{ 0,0 },								/* 9d */
	{ 0,0 },								/* 9e */
	{ "service_action_out_16",service_action_out_16 },					/* 9f */
	{ "report_luns",report_luns },						/* a0 */
	{ "blank",blank },							/* a1 */
	{ "security_protocol_in",security_protocol_in },					/* a2 */
	{ "maintenance_in",maintenance_in },						/* a3 */
	{ "maintenance_out",maintenance_out },						/* a4 */
	{ "move_medium",move_medium },						/* a5 */
	{ "exchange_medium",exchange_medium },						/* a6 */
	{ "move_medium_attached",move_medium_attached },					/* a7 */
	{ "read_12",read_12 },							/* a8 */
	{ "service_action_out_12",service_action_out_12 },					/* a9 */
	{ "write_12",write_12 },							/* aa */
	{ "service_action_in_12",service_action_in_12 },					/* ab */
	{ "erase_12",erase_12 },							/* ac */
	{ "read_dvd_structure",read_dvd_structure },						/* ad */
	{ "write_and_verify_12",write_and_verify_12 },					/* ae */
	{ "verify_12",verify_12 },							/* af */
	{ 0,0 },								/* b0 */
	{ 0,0 },								/* b1 */
	{ 0,0 },								/* b2 */
	{ "set_limits_12",set_limits_12 },						/* b3 */
	{ "read_element_status_attached",read_element_status_attached },				/* b4 */
	{ "security_protocol_out",security_protocol_out },					/* b5 */
	{ "send_volume_tag",send_volume_tag },						/* b6 */
	{ "read_defect_data_12",read_defect_data_12 },					/* b7 */
	{ "read_element_status",read_element_status },					/* b8 */
	{ "read_cd_msf",read_cd_msf },						/* b9 */
	{ "redundancy_group_in",redundancy_group_in },					/* ba */
	{ "redundancy_group_out",redundancy_group_out },					/* bb */
	{ "spare_in",spare_in },							/* bc */
	{ "spare_out",spare_out },							/* bd */
	{ "volume_set_in",volume_set_in },						/* be */
	{ "volume_set_out",volume_set_out },						/* bf */
};

static struct vscsi_op vscsi_pass_op = { "pass-through", vscsi_pass };
	
static int vscsi_queue(struct scsi_cmnd *scp, void (*done)(struct scsi_cmnd *)) {
	struct vscsi_device *dp;
	struct vscsi_op *op;
	int num,byte,mask;

	dprintk("--------------------------------------------------------------------------\n");
	dprintk("vscsi_queue: channel: %d, id: %d, lun: %d\n", scp->device->channel, scp->device->id, scp->device->lun);

	/* Get the device */
	dp = vscsi_get_device(scp->device->channel, scp->device->id, scp->device->lun);
	if ((dp->rom == 0) && (dp->type != TYPE_PASS)) {
		dprintk("vscsi_queue: NO ROM: cmnd[0]: 0x%02x\n", scp->cmnd[0]);
		if (scp->cmnd[0] == INQUIRY) {
			memset(temp,0,96);
			temp[0] = 0x7f;
			temp[3] = 2;
			temp[4] = 92;
			scp->result = response(scp, temp, min(scp->cmnd[4],96));
		} else
			scp->result = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
		done(scp);
		return 0;
	}
//	bindump("dp",dp,sizeof(*dp));

	/* Special op for pass-through devs */
	op = 0;
	if (dp->type == TYPE_PASS) {
		switch(scp->cmnd[0]) {
		case READ_6:
		case READ_10:
		case READ_12:
		case WRITE_6:
		case WRITE_10:
		case WRITE_12:
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
		case READ_16:
		case WRITE_16:
#endif
			op = &vscsi_ops[scp->cmnd[0]];
			break;
		default:
			op = &vscsi_pass_op;
			break;
		}
	} else {
		dprintk("cdb[0]: %x\n", scp->cmnd[0]);
		num = scp->cmnd[0];
		op = (num <= VSCSI_MAX_OP ? &vscsi_ops[num] : 0);
		if (num > VSCSI_MAX_OP) goto bad_cmd;
//		dprintk("num: %d\n", num);
		byte = num / 8;
		mask = 1 << (num % 8);
//		dprintk("byte: %d, mask: %x\n", byte, mask);
//		dprintk("dp: %p\n", dp);
//		dprintk("dp->type: %d\n", dp->type);
		if (dp->type < 0 || dp->type >= VSCSI_NTYPES) {
			/* XXX */
			panic("vscsi_queue: invalid type: %d\n", dp->type);
		}
#if VSCSI_STRICT
		if ((scsi_commands[dp->type][byte] & mask) == 0)
			scp->result = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);
#endif
	}
	if (op) {
		dprintk("enter %s\n", op->name);
		scp->result = op->func(dp, scp);
		dprintk("exit %s\n", op->name);
		goto que_out;
	}

bad_cmd:
	/* XXX check condition */
	scp->result = check_condition(dp, ILLEGAL_REQUEST, INVALID_FIELD_IN_CDB, 0);

que_out:
	dprintk("que_out: result: %d\n", scp->result);
	done(scp);
	return GOOD;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
static int vscsi_config(struct scsi_device *device) {

	dprintk("vscsi_config: id: %d\n", device->id);

//	sdev->use_10_for_rw = 1;
	device->use_10_for_ms = 1;
//	scsi_adjust_queue_depth(sdev, MSG_SIMPLE_TAG, sdev->host->cmd_per_lun);
#if 0
	switch(sdev->type) {
	case TYPE_ROM:
	case TYPE_WORM:
		/* XXX required to get rid of "unaligned transfer" errors */
	        blk_queue_hardsect_size(sdev->request_queue, 2048);
		break;
	case TYPE_DISK:
	default:
		break;
	}
#endif

#if 0
	printk("spi_support_sync: %d\n", spi_support_sync(device->sdev_target));
	printk("spi_support_wide: %d\n", spi_support_wide(device->sdev_target));
	printk("spi_initial_dv: %d\n", spi_initial_dv(device->sdev_target));
	spi_dt(device->sdev_target) = 1;
#endif
	if (spi_support_sync(device->sdev_target) && !spi_initial_dv(device->sdev_target))
		spi_dv_device(device);
	return 0;
}
#endif

static int vscsi_device_reset(struct scsi_cmnd *scp) {
	/* XXX */
	return 0;
}

/* Keep sg size to <= 1 page */
#define VSCSI_SGSIZE ( 4096 / sizeof(struct scatterlist) )
#define VSCSI_CLUSTERING	1
#define VSCSI_DUMP_PARAMS	1

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
struct scsi_host_template vscsi_template = {
	.module			= THIS_MODULE,
	.name			= "Virtual SCSI Adapter",
	.proc_name		= "vscsi",
	.queuecommand		= vscsi_queue,
	.slave_configure	= vscsi_config,
	.skip_settle_delay	= 1,
	.this_id		= -1,
	.sg_tablesize		= VSCSI_SGSIZE,
	.max_sectors		= 0xFFFF,
	.can_queue		= 65535,
	.cmd_per_lun		= 2048,
	.use_clustering		= VSCSI_CLUSTERING,
	.eh_device_reset_handler = vscsi_device_reset,
};
#else
/* XXX 2.4 template */
static void setup_host(struct Scsi_Host *);
static int vscsi_detect(Scsi_Host_Template *shtp) { setup_host(scsi_register(shtp, 0)); return 1; }
static int vscsi_release(struct Scsi_Host *shp) { scsi_unregister(shp); return 0; }
static Scsi_Host_Template vscsi_template = {
	.module			= THIS_MODULE,
	.proc_name		= "vscsi",
//	proc_info:	scsi_debug_proc_info,
	.name			= "Virtual SCSI Adapter",
	.detect			= vscsi_detect,
	.release		= vscsi_release,
//	info:              scsi_debug_info,
//	ioctl:             scsi_debug_ioctl,
	.queuecommand		= vscsi_queue,
//	eh_abort_handler:  scsi_debug_abort,
//	eh_bus_reset_handler: scsi_debug_bus_reset,
	.eh_device_reset_handler = vscsi_device_reset,
//	eh_host_reset_handler: scsi_debug_host_reset,
//	bios_param:        scsi_debug_biosparam,
	can_queue:         255,
	this_id:           7,
	sg_tablesize:      64,
	cmd_per_lun:       3,
	unchecked_isa_dma: 0,
	use_clustering:    VSCSI_CLUSTERING,
	use_new_eh_code:   1,
};
#endif

static void setup_host(struct Scsi_Host *shost) {
	/* Set params */
	shost->max_channel = VSCSI_NUM_CHANS-1;
	shost->max_id = VSCSI_NUM_IDS;
	shost->max_lun = VSCSI_NUM_LUNS;

	shost->io_port = 0;
	shost->n_io_port = 0;
	shost->dma_channel = -1;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	shost->transportt = vscsi_transport_template;
#endif

#if VSCSI_DUMP_PARAMS
#define SDUMP(s,f) dprintk(KERN_INFO "  %16s: %d\n", #f, (s)->f)
	dprintk(KERN_INFO "host parameters:\n");
	SDUMP(shost,max_id);
	SDUMP(shost,max_lun);
	SDUMP(shost,max_channel);
	SDUMP(shost,unique_id);
	SDUMP(&vscsi_template,can_queue);
	SDUMP(&vscsi_template,cmd_per_lun);
	SDUMP(&vscsi_template,sg_tablesize);
	SDUMP(&vscsi_template,max_sectors);
	SDUMP(&vscsi_template,use_clustering);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	SDUMP(shost,use_blk_tcq);
	SDUMP(&vscsi_template,max_host_blocked);
#endif
#undef SDUMP
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static int init_adapter(struct device *dev) {
	struct Scsi_Host *shost;
	int rc;

	/* Get shost */
	dprintk("vSCSI: allocating host...\n");
	shost = scsi_host_alloc(&vscsi_template, sizeof(void *));
	if (!shost) {
		dprintk(KERN_ERR "vSCSI: init_adapter: scsi_host_alloc failed\n");
		return -ENOMEM;
	}
//	dprintk("vSCSI: shost: %p\n", shost);

	/* Setup host parms */
	setup_host(shost);

	/* Add host */
	dprintk("vSCSI: adding host...\n");
	rc = scsi_add_host(shost, dev);
	if (rc) {
		dprintk(KERN_ERR "vSCSI: init_adapter: scsi_add_host failed\n");
		goto err_put;
	}
	dev_set_drvdata(dev, shost);

	/* Scan devs */
	dprintk("vSCSI: Scanning...\n");
	scsi_scan_host(shost);

//	exit(1);
	return 0;

err_put:
	scsi_host_put(shost);
	return rc;
}
#endif

static char *vscsi;

#ifdef MODULE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
module_param(vscsi, charp, 0);
#else
MODULE_PARM(vscsi, "s");
#endif
#else
#endif

static int vscsi_common_setup(char *str) {
	char word[1024],*p;
	int i,type;

	dprintk(">>> vscsi_common_setup: str: %s\n", str);
	type = TYPE_NO_LUN;
	i = 0;
	for(p = str; *p; p++) {
		if (*p == ':' || i >= sizeof(word)-1) {
			word[i] = 0;
			i = 0;
			dprintk("type: %s\n", word);
			if (strcmp(word,"disk")==0)
				type = TYPE_DISK;
			else if (strcmp(word,"cd")==0)
				type = TYPE_ROM;
			else if (strcmp(word,"changer")==0 || strcmp(word,"ch")==0)
				type = TYPE_MEDIUM_CHANGER;
			else if (strcmp(word,"tape")==0)
				type = TYPE_TAPE;
			else if (strcmp(word,"pass")==0)
				type = TYPE_PASS;
			else
				dprintk(KERN_ERR "vSCSI: invalid type: %s\n", word);
		} else if (*p == ',') {
			word[i] = 0;
			i = 0;
			dprintk("path: %s\n", word);
			if (type != TYPE_NO_LUN) {
				vscsi_add_device(type,0,word);
				type = TYPE_NO_LUN;
			}
		} else if (*p != ' ')
			word[i++] = *p;
	}
	if (i) {
		word[i] = 0;
		i = 0;
		dprintk("path: %s\n", word);
		if (type != TYPE_NO_LUN) vscsi_add_device(type,0,word);
	}
	return 1;
}

#ifndef MODULE
static int __init vscsi_setup(char *str)
{
	printk(">>>>> vscsi_setup: str: %s <<<<<\n", str);
	vscsi=str;
	return 0;
}
__setup("vscsi=", vscsi_setup);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static void vscsi_get_signalling(struct Scsi_Host *shost)
{
	spi_signalling(shost) = SPI_SIGNAL_LVD;
}

static struct spi_function_template vscsi_transport_functions =  {
//	.set_period	= vscsi_set_period,
	.show_period	= 1,
//	.set_offset	= vscsi_set_offset,
	.show_offset	= 1,
//	.set_width	= vscsi_set_width,
	.show_width	= 1,
	.get_signalling	= vscsi_get_signalling,
};

static struct platform_device *vscsi_device;

static int __devinit vscsi_probe(struct device *dev)
{
	dprintk("vSCSI: vscsi_probe: probing...\n");

	/* Process vscsi param */
	if (vscsi) vscsi_common_setup(vscsi);

	/* Init the adapter */
	init_adapter(dev);

	return 0;
}

static __devexit int vscsi_device_remove(struct device *dev)
{
        struct Scsi_Host *shost = dev_get_drvdata(dev);

	dprintk("vSCSI: vscsi_device_remove: removing...\n");
	if (shost) {
		scsi_remove_host(shost);
		scsi_host_put(shost);
	}

        return 0;
}

static struct device_driver vscsi_driver = {
        .name   = "vscsi",
        .bus    = &platform_bus_type,
        .probe  = vscsi_probe,
        .remove = __devexit_p(vscsi_device_remove),
};

static int __init vscsi_init(void)
{
        int err;

	vscsi_transport_template = spi_attach_transport(&vscsi_transport_functions);
	if (!vscsi_transport_template) return -ENODEV;

	/* Init devices */
	err = vscsi_device_init();
	if (err) return err;

	dprintk("vSCSI: registering driver...\n");
        err = driver_register(&vscsi_driver);
        if (err) return err;

	dprintk("vSCSI: registering device...\n");
        vscsi_device = platform_device_register_simple("vscsi", -1, NULL, 0);
        if (IS_ERR(vscsi_device)) {
                driver_unregister(&vscsi_driver);
                return PTR_ERR(vscsi_device);
        }

        return err;
}

static void __exit vscsi_exit(void)
{
	spi_release_transport(vscsi_transport_template);
	dprintk("vSCSI: unregistering driver...\n");
	platform_device_unregister(vscsi_device);
	dprintk("vSCSI: unregistering device...\n");
	driver_unregister(&vscsi_driver);
}
#else
static int __init vscsi_init(void)
{
	printk(">>>>> VSCSI INIT <<<<<<<\n");

	/* Init devices */
	if (vscsi_device_init()) return ENODEV;

	/* Process vscsi param */
	if (vscsi) vscsi_common_setup(vscsi);

	/* Register mod */
	scsi_register_module(MODULE_SCSI_HA, &vscsi_template);
	printk("vscsi_template.present: %d\n", vscsi_template.present);
	if (vscsi_template.present) return 0;

	/* Init failed, unreg */
	printk("unregistering and returning ENODEV...\n");
	scsi_unregister_module(MODULE_SCSI_HA, &vscsi_template);
	return -ENODEV;
}

static void __exit vscsi_exit(void)
{
	printk(">>>>> VSCSI EXIT <<<<<<<\n");
	scsi_unregister_module(MODULE_SCSI_HA, &vscsi_template);
}
#endif

//late_initcall(vscsi_init);
module_init(vscsi_init);
module_exit(vscsi_exit);

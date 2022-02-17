
#define DEBUG 1

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/isa.h>
#include <linux/eisa.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <linux/firmware.h>

#include <asm/io.h>
#include <asm/dma.h>

#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsi.h>
#include <scsi/scsi_host.h>

#include "ops.h"
#include "../pci/vpci.h"

#define VSCSI_VERSION_MAJOR 1
#define VSCSI_VERSION_MINOR 0
#define VSCSI_VERSION "1.00"

#ifdef __arch_um__
#include "os.h"
//extern void abort(void);
//extern void exit(int);
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

#if DEBUG
#if 0
static void host_printf(char *fmt, ...) {
	static char msg[2048];
	va_list ap;

	va_start(ap,fmt);
	vsprintf(msg,fmt,ap);
	va_end(ap);
	strcat(msg,"\r");
	os_write_file(2,msg,strlen(msg));
}
#endif
//#define dprintk(format, args...) printk("%s(%d): ",__FUNCTION__,__LINE__), printk(format, ## args)
//#define dprintk(format, args...) host_printf("%s(%d): ",__FUNCTION__,__LINE__), host_printf(format, ## args)
#define dprintk printk
#else /* !DEBUG */
#define dprintk(...) /* noop */
#endif /* DEBUG */

#if 0
#if DEBUG
#ifdef __arch_um__
static void host_printf(char *fmt, ...) {
	char msg[1024];
	va_list ap;

	va_start(ap,fmt);
	vsprintf(msg,fmt,ap);
	va_end(ap);
	strcat(msg,"\r");
	os_write_file(2,msg,strlen(msg));
}
#define dprintk host_printf
#else /* !__arch_um__ */
#define dprintk printk
#endif /* __arch_um__ */
#else /* !DEBUG */
#define dprintk(...) /* noop */
#endif /* DEBUG */
#endif

#define ENTER dprintk("entering");
#define EXIT dprintk("exiting");

static unsigned char temp[192];

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

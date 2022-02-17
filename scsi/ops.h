struct vscsi_device;
typedef int (*vscsi_opfunc_t)(struct vscsi_device *, struct scsi_cmnd *);
typedef vscsi_opfunc_t vscsi_ops_t;
#define HAVE_DISK 1
static vscsi_ops_t vscsi_disk_ops[];

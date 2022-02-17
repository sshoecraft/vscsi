
struct vscsi_file_ops;

/* file pointer */
struct vscsi_file {
	void *fd;			/* Host/Local file desc */
	char path[1024];		/* Path to file */
	unsigned long long size;	/* File size, in bytes */
	struct vscsi_file_ops *ops;	/* File ops */
};

/* file ops */
struct vscsi_file_ops {
	int (*open)(struct vscsi_file *, int);
	int (*close)(struct vscsi_file *);
	int (*rw)(struct vscsi_file *, struct scsi_cmnd *, unsigned long long, int, int);
	int (*pass)(struct vscsi_file *, struct scsi_cmnd *);
};

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

lo->lo_thread = kthread_create(loop_thread, lo, "loop%d", lo->lo_number);
if (IS_ERR(lo->lo_thread)) {
	error = PTR_ERR(lo->lo_thread);
	goto out_clr;
}

#endif

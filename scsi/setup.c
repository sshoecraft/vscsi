
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


static char *vnet;

#ifdef MODULE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
module_param(vnet, charp, 0);
#else
MODULE_PARM(vnet, "s");
#endif
#else
#endif

static int vnet_common_setup(char *str) {
	char word[1024],*p;
	int i,type;

	dprintk(">>> vnet_common_setup: str: %s\n", str);
//	type = TYPE_NO_LUN;
	i = 0;
	vpci_add_device(0, PCI_DEVICE_ID_VNET, PCI_CLASS_NETWORK_ETHERNET, 0);
	for(p = str; *p; p++) {
#if 0
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
				vnet_add_device(type,0,word);
				type = TYPE_NO_LUN;
			}
		} else if (*p != ' ')
			word[i++] = *p;
#endif
	}
	if (i) {
		word[i] = 0;
		i = 0;
		dprintk("path: %s\n", word);
//		if (type != TYPE_NO_LUN) vnet_add_device(type,0,word);
	}
	return 1;
}

#ifndef MODULE
static int __init vnet_setup(char *str)
{
	printk(">>>>> vnet_setup: str: %s <<<<<\n", str);
	vnet=str;
	return 0;
}
__setup("vnet=", vnet_setup);
#endif

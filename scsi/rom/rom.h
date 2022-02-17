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


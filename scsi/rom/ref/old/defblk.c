
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/mtio.h>

int main(void) {
	int fd,bs;
	struct mtop op = { ZZ

	fd = open("/dev/st0",O_RDONLY);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	bs=128*1024;
	if (ioctl(fd,MT_ST_DEF_BLKSIZE,&bs) < 0)
		perror("ioctl");
	close(fd);
	return 0;
}

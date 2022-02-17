
#include <stdio.h>

int main(int argc, char **argv) {
	char path[1024], *config;

	if (argc < 2) {
		printf("usage: mkdrv <path>");
		return 1;
	}
	config = argv[1];
	printf("config: %s\n", config);

	return 0;
}

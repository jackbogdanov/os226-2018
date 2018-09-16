#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "init.h"

#include "hal/dbg.h"
#include "hostexn.h"

static void *read_rootfs(const char *cpio) {
	struct stat st;
	if (-1 == stat(cpio, &st)) {
		perror("can't stat rootfs cpio");
		return NULL;
	}

	void *rootfs = malloc(st.st_size);
	if (!rootfs) {
		fprintf(stderr, "can't alloc\n");
		return NULL;
	}

	int fd = open(cpio, 0);
	if (-1 == fd) {
		perror("can't open rootfs cpio");
		return NULL;
	}

	if (st.st_size != read(fd, rootfs, st.st_size)) {
		fprintf(stderr, "truncated file\n");
		return NULL;
	}

	close(fd);
	return rootfs;
}

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		fprintf(stderr, "no rootfs.cpio provided\n");
		return 1;
	}

	void *rootfs = read_rootfs(argv[1]);
	if (!rootfs) {
		return 1;
	}

	const size_t memsz = 0x1000000;
	void *mem = mmap(NULL, memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (MAP_FAILED == mem) {
		perror("can't alloc mem");
		return 1;
	}

	kernel_init(rootfs, mem, memsz, "");
	exn_init();
	kernel_start();
}

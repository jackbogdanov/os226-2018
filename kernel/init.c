
#include <stddef.h>
#include <stdbool.h>

#include "init.h"
#include "time.h"
#include "palloc.h"
#include "hal/dbg.h"
#include "ksys.h"

int kernel_init(void *rootfs_cpio, void *mem, size_t sz, const char* args) {
	int r;
	if ((r = rootfs_cpio_init(rootfs_cpio))) {
		return r;
	}

	if ((r = palloc_init(mem, sz))) {
		return r;
	}

	return 0;
}

void kernel_start(void) {
	if (time_init()) {
		panic("time failed");
	}

	char *argv[] = { "shell", NULL };
	if (sys_run(argv) < 0) {
		panic("first process failed");
	}

	if (sched_init()) {
		panic("sched_init failed");
	}

	while (true) {
		sched(true);
	}
}

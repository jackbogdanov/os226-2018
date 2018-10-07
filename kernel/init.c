
#include <stddef.h>
#include <stdbool.h>

#include "init.h"
#include "hal/dbg.h"
#include "ksys.h"

struct kernel_globals kernel_globals;

void kernel_init(void *rootfs_cpio, void *mem, size_t sz, const char* args) {
	kernel_globals.rootfs_cpio = rootfs_cpio;
	kernel_globals.mem = mem;
	kernel_globals.memsz = sz;
}

void kernel_start(void) {

	char *argv[] = { "shell", NULL };
	if (sys_run(argv) < 0) {
		panic("first process failed");
	}

	if (sched_init()) {
		panic("sched_init failed");
	}

	while (true) {
		sched();
	}
}

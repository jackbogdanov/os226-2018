
#include <stddef.h>

#include "init.h"
#include "hal/dbg.h"
#include "ksys.h"

struct kernel_globals kernel_globals;

void kernel_init(void *rootfs_cpio, void *mem, size_t sz, const char* args) {
	kernel_globals.rootfs_cpio = rootfs_cpio;
	kernel_globals.mem = mem;
	kernel_globals.mem_ptr = mem;
	kernel_globals.block_ptr = 0;
	kernel_globals.memsz = sz;
}

void kernel_start(void) {
	char *argv[] = { "shell", NULL };
	void *entry;
	void *mark = load(argv[0], &entry);
	if (mark) {
		((int (*)(int, char **))entry)(1, argv);
		unload(mark);
	}

	hal_halt();
	while (1);
}

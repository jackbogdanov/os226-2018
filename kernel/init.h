#pragma once

#include "util.h"

struct kernel_globals {
	void *rootfs_cpio;
	void *mem;
	size_t memsz;
};

extern struct kernel_globals kernel_globals;

void kernel_init(void *rootfs_cpio, void *mem, size_t sz, const char* args);

void kernel_start(void) NORETURN;

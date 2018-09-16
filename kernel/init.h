#pragma once

#include "util.h"

#define CURRENT_SIZE_PTR_OFFSET -1
#define PREVIOUS_SIZE_PTR_OFFSET -2

struct kernel_globals {
	void *rootfs_cpio;
	void *mem;
	void *mem_ptr;
	size_t block_ptr;
	size_t block_sizes[20];
	size_t memsz;
};

extern struct kernel_globals kernel_globals;

void kernel_init(void *rootfs_cpio, void *mem, size_t sz, const char* args);

void kernel_start(void) NORETURN;

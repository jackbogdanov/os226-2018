#pragma once

#include "util.h"

#define MAX_TASKS_IN_QUEUE 20

struct kernel_globals {
	void *rootfs_cpio;
	void *mem;
	size_t memsz;
	char **tasks[MAX_TASKS_IN_QUEUE];
};

extern struct kernel_globals kernel_globals;

void kernel_init(void *rootfs_cpio, void *mem, size_t sz, const char* args);

void kernel_start(void) NORETURN;

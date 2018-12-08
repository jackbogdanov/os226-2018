#pragma once

struct context;

unsigned long syscall_do(struct context *ctx, int sysnum,
		unsigned long arg1, unsigned long arg2,
		unsigned long arg3, unsigned long arg4,
		void *rest);

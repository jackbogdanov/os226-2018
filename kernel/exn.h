#pragma once

#include <stdbool.h>

struct context;

struct exn_ctx {
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long rdi;
	unsigned long rsi;
	unsigned long rdx;
	unsigned long rcx;
	unsigned long rbx;
	unsigned long rax;
	unsigned long rflags;
	unsigned long target;
	unsigned long sp;
	unsigned long saved_rbp;
	unsigned long exn;
	unsigned long rip;
};

typedef bool (*exn_hnd_t)(int exn, struct context *c, void *arg);

int exn_set_hnd(int exn, exn_hnd_t h, void *arg);

void exn_do(int exn, struct context *c);

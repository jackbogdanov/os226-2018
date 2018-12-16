#define _GNU_SOURCE

#include <stddef.h>

#include "hal/context.h"
#include "hal_context.h"
#include "util.h"
#include "exn.h"

#include <string.h>

void ctx_make(struct uctx *ctx, void *entry, void *stack, int stacksz) {
	memset(ctx, 0, sizeof(*ctx));

	ctx->rsp = (unsigned long) stack + stacksz - 16;
	ctx->rsp = ctx->rsp;
	ctx->rsp -= 8;
	*(unsigned long *)ctx->rsp = (unsigned long) entry;
}

void ctx_copy(struct context *old_ctx, struct context *ctx) {
	struct exn_ctx *new_ctx = (struct exn_ctx *) old_ctx;
	memset(new_ctx, 0, sizeof(*new_ctx));
	struct exn_ctx *cur_ctx = (struct exn_ctx *) ctx;
	
	new_ctx->sp = cur_ctx->sp;

	new_ctx->rbx = cur_ctx->rbx;
	new_ctx->r12 = cur_ctx->r12;
	new_ctx->r13 = cur_ctx->r13;
	new_ctx->r14 = cur_ctx->r14;
	new_ctx->r15 = cur_ctx->r15;
	new_ctx->r11 = cur_ctx->r11;
	new_ctx->r10 = cur_ctx->r10;
	new_ctx->r9 = cur_ctx->r9;
	new_ctx->r8 = cur_ctx->r8;

	new_ctx->rdi = cur_ctx->rdi;
	new_ctx->rsi = cur_ctx->rsi;
	new_ctx->rdx = cur_ctx->rdx;
	new_ctx->rcx = cur_ctx->rcx;
	new_ctx->saved_rbp = cur_ctx->saved_rbp;
	new_ctx->rflags = cur_ctx->rflags;
	new_ctx->target = cur_ctx->target;
	new_ctx->exn = cur_ctx->exn;
	new_ctx->rip = cur_ctx->rip;

	new_ctx->rax = 0;
}


void push_to_stack(unsigned long* sp, unsigned long value, unsigned long stack) {
	*((unsigned long*)stack) = value;
	*(sp) -= sizeof(unsigned long);
}

void prepare_stack(struct proc *newp, struct proc *curp) {
	newp->ctx.rsp = newp->full_ctx.sp;
	newp->ctx.rip = newp->full_ctx.rip;

	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rip, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.sp, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rflags, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.saved_rbp, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r15, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r14, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r13, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r12, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r11, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r10, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r9, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.r8, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rdi, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rsi, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rdx, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rcx, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rbx, newp->ctx.rsp);
	push_to_stack(&(newp->ctx.rsp), newp->full_ctx.rax, newp->ctx.rsp);
	newp->ctx.rsp += (sizeof(unsigned long));
}

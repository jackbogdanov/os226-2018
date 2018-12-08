#define _GNU_SOURCE

#include <stddef.h>


#include "hal/context.h"
#include "hal_context.h"
#include "hal/dbg.h"
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

void ctx_copy(struct uctx *new_ctx, struct context *_ctx, int delta) {
	memset(new_ctx, 0, sizeof(*new_ctx));
	struct exn_ctx *cur_ctx = (struct exn_ctx *) _ctx;
	dbg_out("copyping\n", 9);
	new_ctx->rsp = cur_ctx->sp;// + (unsigned long) delta;

	dbg_out("finished\n", 9);	

	new_ctx->rbx = cur_ctx->rbx;
	new_ctx->r12 = cur_ctx->r12;
	new_ctx->r13 = cur_ctx->r13;
	new_ctx->r14 = cur_ctx->r14;
	new_ctx->r15 = cur_ctx->r15;
	//new_ctx->rbp = cur_ctx->rbp;
}

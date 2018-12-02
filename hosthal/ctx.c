#define _GNU_SOURCE

#include <stddef.h>

#include "hal/context.h"
#include "hal_context.h"
#include "util.h"

#include <string.h>

void ctx_make(struct uctx *ctx, void *entry, void *stack, int stacksz) {
	memset(ctx, 0, sizeof(*ctx));

	ctx->rsp = (unsigned long) stack + stacksz - 16;
	ctx->rsp = ctx->rsp;
	ctx->rsp -= 8;
	*(unsigned long *)ctx->rsp = (unsigned long) entry;
}

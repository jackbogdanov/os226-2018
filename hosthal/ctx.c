#define _GNU_SOURCE

#include <stddef.h>

#include "hal/context.h"
#include "hal_context.h"
#include "util.h"

#include <ucontext.h>
#include <string.h>

STATIC_ASSERT(sizeof(ucontext_t) <= HOST_UCTX_SZ);

static void proctramp(void *_entry) {
	void (*entry)() = _entry;
	entry();
}

void ctx_make(struct uctx *_ctx, void *entry, void *stack, int stacksz) {
	ucontext_t *ctx = (ucontext_t *) _ctx;
	memset(ctx, 0, sizeof(*ctx));
	getcontext(ctx);
	ctx->uc_stack.ss_sp = stack + stacksz - 16;
	ctx->uc_stack.ss_size = 0;
	makecontext((ucontext_t *)ctx, (void *) proctramp, 1, entry);
}

void ctx_switch(struct uctx *old, struct uctx *new) {
	swapcontext((ucontext_t *) old, (ucontext_t *) new);
}

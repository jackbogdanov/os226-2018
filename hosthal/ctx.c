#define _GNU_SOURCE

#include <stddef.h>

#include "hal/context.h"
#include "hal_context.h"
#include "util.h"

#include <ucontext.h>
#include <string.h>

STATIC_ASSERT(sizeof(ucontext_t) <= HOST_UCTX_SZ);

extern void tramptramp(void);

static ucontext_t *h2uctx(struct context *hctx) {
	return (ucontext_t *) &hctx->host_uctx;
}

static greg_t *h2gregs(struct context *ctx) {
	return h2uctx(ctx)->uc_mcontext.gregs;
}

void ctx_push(struct context *ctx, unsigned long val) {
	greg_t *regs = h2uctx(ctx)->uc_mcontext.gregs;
	regs[REG_RSP] -= sizeof(unsigned long);
	*(unsigned long *) regs[REG_RSP] = val;
}

void ctx_call_setup(struct context *ctx, void(*tramp)(unsigned long *), struct context_call_save *save) {
	greg_t *regs = h2uctx(ctx)->uc_mcontext.gregs;

	ctx_push(ctx, regs[REG_RIP]);
	regs[REG_RIP] = (greg_t) tramptramp;

	save->spbeforearg = (unsigned long) regs[REG_RSP];
	save->calltraget = (unsigned long) tramp;
}

void ctx_call_end(struct context *ctx, struct context_call_save *save) {
	ctx_push(ctx, save->spbeforearg);
	ctx_push(ctx, save->calltraget);
}

void ctx_save(struct context *ctx, struct context *save, void *entry, void *stack, int stacksz) {
	greg_t *regs = h2gregs(ctx);
	greg_t *regsave = h2gregs(save);

	memcpy(regsave, regs, sizeof(gregset_t));
	regs[REG_RSP] = (greg_t) stack + stacksz - 16;
	regs[REG_RBP] = (greg_t) regs[REG_RSP];
	regs[REG_RIP] = (greg_t) entry;
	// FIXME clear other regs
}

void ctx_restore(struct context *ctx, struct context *save) {
	memcpy(h2gregs(ctx), h2gregs(save), sizeof(gregset_t));
}

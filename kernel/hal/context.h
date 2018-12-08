#pragma once

struct uctx;
struct context;

extern void ctx_make(struct uctx *ctx, void *entry, void *stack, int stacksz);
extern void ctx_switch(struct uctx *old, struct uctx *new);
extern void ctx_copy(struct uctx *new_ctx, struct context *cur_ctx, int delta);

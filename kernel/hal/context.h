#pragma once

struct uctx;

extern void ctx_make(struct uctx *ctx, void *entry, void *stack, int stacksz);
extern void ctx_switch(struct uctx *old, struct uctx *new);

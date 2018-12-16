#pragma once
#include "exn.h"
#include "proc.h"

struct uctx;
struct context;
struct exn_ctx;

extern void ctx_make(struct uctx *ctx, void *entry, void *stack, int stacksz);
extern void ctx_copy(struct context *new_ctx, struct context *ctx);

void push_to_stack(unsigned long* sp, unsigned long value, unsigned long stack);
void prepare_stack(struct proc *newp, struct proc *curp);

extern void ctx_switch(struct uctx *old, struct uctx *new);
extern void ctx_switch_fork(struct uctx *old, struct uctx *new);

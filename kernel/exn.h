#pragma once

#include <stdbool.h>

#define PROGRAM_EXN_HND 40

struct context;

typedef bool (*exn_hnd_t)(int exn, struct context *c, void *arg);

int exn_set_hnd(int exn, exn_hnd_t h, void *arg);

void exn_do(int exn, struct context *c);

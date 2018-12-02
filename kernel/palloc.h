#pragma once

#include <stddef.h>

#define PSHFT 12
#define PSIZE (1 << PSHFT)

static inline int psize(size_t sz) {
	return (sz + PSIZE - 1) >> PSHFT;
}

int palloc_init(void *mem, size_t memsz);

void *palloc(int n);

void pfree(void *p, int n);

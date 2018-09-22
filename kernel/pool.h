#pragma once

#include "third-party/queue.h"
#include "util.h"

struct pool_free_block {
	SLIST_ENTRY(pool_free_block) next;
};

struct pool {
	char *mem;
	unsigned long membsz;
	char *freestart;
	char *freeend;
	SLIST_HEAD(pool_free_queue, pool_free_block) freehead;
};

#define POOL_INITIALIZER(_pool, _mem, _nmemb, _membsz) { \
	.mem = (char*)(_mem), \
	.membsz	= (_membsz), \
	.freehead = SLIST_HEAD_INITIALIZER(&(_pool)->freehead), \
	.freestart = (char*)(_mem), \
	.freeend = (char*)(_mem) + (_nmemb) * (_membsz), \
}

#define POOL_INITIALIZER_ARRAY(_pool, _array) \
	POOL_INITIALIZER(_pool, _array, ARRAY_SIZE(_array), sizeof((_array)[0]));

void pool_init(struct pool *p, void *mem, unsigned long nmemb, unsigned long membsz);

void *pool_alloc(struct pool *p);

void pool_free(struct pool *p, void *ptr);

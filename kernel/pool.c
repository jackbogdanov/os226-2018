
#include <stddef.h>

#include "pool.h"

void pool_init(struct pool *p, void *mem, unsigned long nmemb, unsigned long membsz) {
	p->mem = mem;
	p->membsz = membsz;
	p->freestart = mem;
	p->freeend = p->freestart + nmemb * membsz;
	SLIST_INIT(&p->freehead);
}

void *pool_alloc(struct pool *p) {
	if (p->freestart < p->freeend) {
		void *r = p->freestart;
		p->freestart += p->membsz;
		return r;
	}

	struct pool_free_block *fb = SLIST_FIRST(&p->freehead);
	if (fb) {
		SLIST_REMOVE_HEAD(&p->freehead, next);
		return fb;
	}

	return NULL;
}

void pool_free(struct pool *p, void *ptr) {
	SLIST_INSERT_HEAD(&p->freehead, (struct pool_free_block*)ptr, next);
}

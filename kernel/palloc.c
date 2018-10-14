
#include "palloc.h"
#include "init.h"
#include "third-party/queue.h"

struct freepage {
	SLIST_ENTRY(freepage) entry;
	int n;
};

static SLIST_HEAD(freehead, freepage) freepages = SLIST_HEAD_INITIALIZER(freepages);

int palloc_init(void *mem, size_t memsz) {
	struct freepage *fp = mem;
	fp->n = memsz >> PSHFT;
	SLIST_INSERT_HEAD(&freepages, fp, entry);
	return 0;
}

void *palloc(int n) {
	struct freepage *ifp, **prevfp;
	SLIST_FOREACH_PREVPTR(ifp, prevfp, &freepages, entry) {
		if (n == ifp->n) {
			SLIST_REMOVE_PREVPTR(prevfp, ifp, entry);
			return ifp;
		}
		if (n < ifp->n) {
			ifp->n -= n;
			return ((char*)ifp) + PSIZE * ifp->n;
		}
	}
	return NULL;
}

void pfree(void *p, int n) {
	struct freepage *ifp;
	struct freepage *left = NULL;
	if ((void*)SLIST_FIRST(&freepages) < p) {
		SLIST_FOREACH(ifp, &freepages, entry) {
			struct freepage *next = SLIST_NEXT(ifp, entry);
			if (!next || (void*)p < (void*)next) {
				left = ifp;
				break;
			}
		}
	}

	struct freepage *m;
	if (left && ((char*)left) + PSIZE * left->n == p) {
		left->n += n;
		m = left;
	} else {
		struct freepage *newfp = p;
		newfp->n = n;
		if (left) {
			SLIST_INSERT_AFTER(left, newfp, entry);
		} else {
			SLIST_INSERT_HEAD(&freepages, newfp, entry);
		}
		m = newfp;
	}

	struct freepage *right = SLIST_NEXT(m, entry);
	if (right && ((char*)m) + PSIZE * m->n == ((char*)right)) {
		m->n += right->n;
		SLIST_REMOVE_AFTER(m, entry);
	}
}

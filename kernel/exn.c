
#include "util.h"
#include "exn.h"
#include "ksys.h"

#define EXN_N_MAX 64

static struct exndes {
	exn_hnd_t hnd;
	void* arg;
} exn_hnd_table[EXN_N_MAX];

int exn_set_hnd(int exn, exn_hnd_t h, void *arg) {
	if (exn < 0 || EXN_N_MAX <= exn) {
		return -1;
	}
	struct exndes *e = &exn_hnd_table[exn];
	e->hnd = h;
	e->arg = arg;

	return 0;
}

void exn_do(int exn, struct context *c) {
	assert(0 <= exn && exn < EXN_N_MAX);
	struct exndes *e = &exn_hnd_table[exn];

	if (!e->hnd) {
		return;
	}

	e->hnd(exn, c, e->arg);
	sched_handle_posted(c);
}

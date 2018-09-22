
#include "util.h"
#include "exn.h"
#include "hal/dbg.h"

#define EXN_N_MAX 64

static struct exndes {
	exn_hnd_t hnd;
	void* arg;
} exn_hnd_table[EXN_N_MAX];

int exn_set_hnd(int exn, exn_hnd_t h, void *arg) {

	if (exn == 40) {
		dbg_out("echo setting hnd\n", 17);
	}

	if (exn < 0 || EXN_N_MAX <= exn) {
		return -1;
	}
	struct exndes *e = &exn_hnd_table[exn];
	e->hnd = h;
	e->arg = arg;

	return 0;
}

void exn_do(int exn, struct context *c) {
	if (exn == 40) {
		dbg_out("echo doing hnd\n", 15);
	}

	assert(0 <= exn && exn < EXN_N_MAX);
	struct exndes *e = &exn_hnd_table[exn];

	if (!e->hnd) {
		return;
	}

	e->hnd(exn, c, e->arg);
}

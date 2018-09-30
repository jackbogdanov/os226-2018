
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "exn.h"
#include "hosttime.h"

static bool timer_hnd(int exn, struct context *c, void *arg) {
	// IMPL ME
	return true;
}

int time_init(void) {
	int res;

	if ((res = exn_set_hnd(SIGALRM, timer_hnd, NULL))) {
		return res;
	}

	const struct timeval initv = {
		.tv_sec  = 1,
		.tv_usec = 0,
	};
	const struct itimerval setup_it = {
		.it_value    = initv,
		.it_interval = initv,
	};
	if (-1 == setitimer(ITIMER_REAL, &setup_it, NULL)) {
		perror("SIGALRM set failed");
		return -1;
	}
	return 0;
}

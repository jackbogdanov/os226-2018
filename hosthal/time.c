
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>

#include "exn.h"

static struct timeval initv;

int hal_timer_init(int msec, bool reload, exn_hnd_t hnd, void *arg) {
	return 0; // FIXME

	initv.tv_sec  = msec / 1000;
	initv.tv_usec = msec * 1000;

	const struct itimerval setup_it = {
		.it_value    = initv,
		.it_interval = initv,
	};
	if (-1 == setitimer(ITIMER_REAL, &setup_it, NULL)) {
		perror("SIGALRM set failed");
		return -1;
	}

	return exn_set_hnd(SIGALRM, hnd, arg);
}

int hal_timer_counter(void) {
        struct itimerval it;
        getitimer(ITIMER_REAL, &it);
        return 1000000 * (initv.tv_sec - it.it_value.tv_sec)
		+ (initv.tv_usec - it.it_value.tv_usec);
}

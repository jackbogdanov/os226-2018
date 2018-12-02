#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "hal/dbg.h"

void dbg_out(const char *msg, int len) {
	write(1, msg, len);
}

int dbg_in(char *inp, int len) {
	int r = read(STDIN_FILENO, inp, len);
	return r;
}

void hal_halt(void) {
	exit(0);
}

int dbg_pause(void) {
	pause();
	return 0;
}

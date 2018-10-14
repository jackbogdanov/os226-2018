
#include <stddef.h>
#include "time.h"
#include "hal/timer.h"

static int current_ms;

static bool timer_hnd(int exn, struct context *c, void *arg) {
	++current_ms;
	return true;
}

int time_init(void) {
	return hal_timer_init(1, true, timer_hnd, NULL);
}

int time_current(void) {
	return current_ms * 1000 + hal_timer_counter();
}

int timer_init(int ms, void(*fn)(void *arg), void *arg) {
	return -1;
}

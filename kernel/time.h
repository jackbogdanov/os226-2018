#pragma once

#include <stdbool.h>
#include "third-party/queue.h"

struct timer {
	SLIST_ENTRY(timer) entry;
	void(*hnd)(void *arg);
	void *arg;
	int ms;
	int diff;
	bool reload;
};

int time_init(void);

int time_current(void);

int timer_init(struct timer *t, int ms, bool reload, void(*fn)(void *arg), void *arg);

int timer_start(struct timer *t);

int timer_stop(struct timer *t);

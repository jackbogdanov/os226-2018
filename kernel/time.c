
#include <stddef.h>
#include "third-party/queue.h"
#include "exn.h"
#include "util.h"
#include "time.h"
#include "hal/timer.h"

static SLIST_HEAD(timerqhead, timer) timerq = SLIST_HEAD_INITIALIZER(timerq);

static int current_ms;

static bool timer_started(struct timer *t) {
	return 0 <= t->diff;
}

int timer_init(struct timer *t, int ms, bool reload, void(*fn)(void *arg), void *arg) {
	t->hnd = fn;
	t->arg = arg;
	t->ms = ms;
	t->reload = reload;
	t->diff = -1;
	return 0;
}

int timer_start(struct timer *t) {
	bool irq = irq_save();
	if (timer_started(t)) {
		goto started;
	}

	t->diff = t->ms;
	struct timer *after = NULL;
	struct timer *it;
	SLIST_FOREACH(it, &timerq, entry) {
		if (t->diff < it->diff) {
			break;
		}
		t->diff -= it->diff;
		after = it;
	}

	if (after) {
		SLIST_INSERT_AFTER(after, t, entry);
	} else {
		SLIST_INSERT_HEAD(&timerq, t, entry);
	}

	assert(timer_started(t));
started:
	irq_restore(irq);
	return 0;
}

int timer_stop(struct timer *t) {
	bool irq = irq_save();
	if (!timer_started(t)) {
		goto stopped;
	}

	struct timer *it, **prev;
	SLIST_FOREACH_PREVPTR(it, prev, &timerq, entry) {
		if (it == t) {
			SLIST_REMOVE_PREVPTR(prev, it, entry);
			struct timer *next = SLIST_NEXT(it, entry);
			if (next) {
				next->diff += it->diff;
			}
		}
	}

	t->diff = -1;
stopped:
	irq_restore(irq);
	return 0;
}

static bool timer_hnd(int exn, struct context *c, void *arg) {
	bool irq = irq_save();

	++current_ms;

	struct timer *t = SLIST_FIRST(&timerq);
	if (t) {
		assert(0 < t->diff);
		--t->diff;

		while (t && !t->diff) {
			SLIST_REMOVE_HEAD(&timerq, entry);
			t->hnd(t->arg);
			if (t->reload) {
				timer_start(t);
			}
			t = SLIST_FIRST(&timerq);
		}
	}

	irq_restore(irq);
	return true;
}

int time_current(void) {
	return current_ms * 1000 + hal_timer_counter();
}

int time_init(void) {
	return hal_timer_init(1, true, timer_hnd, NULL);
}


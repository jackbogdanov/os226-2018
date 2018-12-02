#pragma once

#include <stdbool.h>

#include "exn.h"

int hal_timer_init(int msec, bool reload, exn_hnd_t, void *arg);

int hal_timer_counter(void);

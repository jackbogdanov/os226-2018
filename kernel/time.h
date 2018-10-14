#pragma once

int time_init(void);

int time_current(void);

int timer_init(int ms, void(*fn)(void *arg), void *arg);

#pragma once

void dbg_out(const char *msg, int len);

int dbg_in(char *inp, int len);

void hal_halt(void);

int dbg_pause(void);

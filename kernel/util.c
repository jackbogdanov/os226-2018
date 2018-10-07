
#include "string.h"
#include "stdio.h"
#include "hal/dbg.h"

void failed_assert(const char *cond, const char *file, int line) {
	char msg[128];
	int len = snprintf(msg, sizeof(msg), "assert failed %s:%d\n\t%s\n", file, line, cond);
	dbg_out(msg, len);
	hal_halt();
}

void panic(const char *fmt, ...) {
	char msg[128];
	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	dbg_out(msg, len);
	hal_halt();
}

void kprint(const char *fmt, ...) {
	char msg[128];
	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	dbg_out(msg, len);
}

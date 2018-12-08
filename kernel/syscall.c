
#include "ksys.h"
#include "hal/context.h"
#include "syscall.h"
#include "util.h"

typedef unsigned long(*sys_call_t)(struct context *ctx,
		unsigned long arg1, unsigned long arg2,
		unsigned long arg3, unsigned long arg4,
		void *rest);

#define TABLE_LIST(name, ...) (sys_call_t) sys_ ## name,
static const sys_call_t sys_table[] = {
	SYSCALL_XN(TABLE_LIST)
};
#undef TABLE_LIST

unsigned long syscall_do(struct context *ctx, int sysnum,
		unsigned long arg1, unsigned long arg2,
		unsigned long arg3, unsigned long arg4,
		void *rest) {
	return sys_table[sysnum](ctx, arg1, arg2, arg3, arg4, rest);
}

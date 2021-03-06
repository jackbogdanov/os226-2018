#pragma once

#include <stdbool.h>

#include "kernel/syscall.h"

struct context;

#define DO_DECLARE(ret, name, ...) \
	extern ret sys_ ## name(__VA_ARGS__)
#define DECLARE0(ret, name) \
	DO_DECLARE(ret, name);
#define DECLARE1(ret, name, type1, name1) \
	DO_DECLARE(ret, name, type1);
#define DECLARE2(ret, name, type1, name1, type2, name2) \
	DO_DECLARE(ret, name, type1, type2);
#define DECLARE3(ret, name, type1, name1, type2, name2, type3, name3) \
	DO_DECLARE(ret, name, type1, type2, type3);
#define DECLARE4(ret, name, type1, name1, type2, name2, type3, name3, type4, name4) \
	DO_DECLARE(ret, name, type1, type2, type3, type4);
#define DECLARE5(ret, name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5) \
	DO_DECLARE(ret, name, type1, type2, type3, type4, type5);
#define DECLARE(name, ret, n, ...) \
	DECLARE ## n (ret, name, ## __VA_ARGS__)
SYSCALL_XN(DECLARE)
#undef DECLARE0
#undef DECLARE1
#undef DECLARE2
#undef DECLARE3
#undef DECLARE4
#undef DECLARE5
#undef DECLARE
#undef DO_DECLARE

extern int rootfs_cpio_init(void *p);

extern int sched_init(void);

extern void sched(bool voluntary);

extern void sched_handle_posted(void);

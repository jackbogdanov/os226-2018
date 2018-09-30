#pragma once

#include <stddef.h>

typedef void (*tramp_t)(void *arg);

#define SYSCALL_XN(x) \
	x(run, int, 2, char**, argv, int*, exitcode) \
	x(getargv, int, 4, char *, buf, int, bufsz, char **, argv, int, argvsz) \
	x(exit, int, 1, int, exitcode) \
	x(write, int, 3, int, fd, const void *, buf, size_t, len) \
	x(read, int, 3, int, fd, void *, buffer, size_t, size) \
	x(sleep, int, 1, int, msec) \
	x(uptime, int, 0) \

#define ENUM_LIST(name, ...) os_syscall_nr_ ## name,
enum syscalls_num {
	SYSCALL_XN(ENUM_LIST)
};
#undef ENUM_LIST

static inline long os_syscall(int syscall,
		unsigned long arg1, unsigned long arg2,
		unsigned long arg3, unsigned long arg4,
		void *rest) {
	long ret;
	__asm__ __volatile__(
		"int $0x81\n"
		: "=a"(ret)
		: "a"(syscall), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(rest)
		:
	);
	return ret;
}

#define DEFINE0(ret, name) \
	static inline ret os_ ## name () { \
		return (ret) os_syscall(os_syscall_nr_ ## name, 0, 0, 0, 0, (void *) 0); \
	}
#define DEFINE1(ret, name, type1, name1) \
	static inline ret os_ ## name (type1 name1) { \
		return (ret) os_syscall(os_syscall_nr_ ## name, (unsigned long) name1, 0, 0, 0, (void *) 0); \
	}
#define DEFINE2(ret, name, type1, name1, type2, name2) \
	static inline ret os_ ## name (type1 name1, type2 name2) { \
		return (ret) os_syscall(os_syscall_nr_ ## name, (unsigned long) name1, (unsigned long) name2, 0, 0, (void *) 0); \
	}
#define DEFINE3(ret, name, type1, name1, type2, name2, type3, name3) \
	static inline ret os_ ## name (type1 name1, type2 name2, type3 name3) { \
		return (ret) os_syscall(os_syscall_nr_ ## name, (unsigned long) name1, (unsigned long) name2, \
				(unsigned long) name3, 0, (void *) 0); \
	}
#define DEFINE4(ret, name, type1, name1, type2, name2, type3, name3, type4, name4) \
	static inline ret os_ ## name (type1 name1, type2 name2, type3 name3, type4 name4) { \
		return (ret) os_syscall(os_syscall_nr_ ## name, (unsigned long) name1, (unsigned long) name2, \
				(unsigned long) name3, (unsigned long) name4, (void *) 0); \
	}
#define DEFINE5(ret, name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5) \
	static inline ret os_ ## name (type1 name1, type2 name2, type3 name3, type4 name4) { \
		return (ret) os_syscall(os_syscall_nr_ ## name, (unsigned long) name1, (unsigned long) name2, \
				(unsigned long) name3, (unsigned long) name4, (void *) name5); \
	}
#define DEFINE(name, ret, n, ...) \
	DEFINE ## n (ret, name, ## __VA_ARGS__)
SYSCALL_XN(DEFINE)
#undef DEFINE0
#undef DEFINE1
#undef DEFINE2
#undef DEFINE3
#undef DEFINE4
#undef DEFINE5
#undef DEFINE

#define _GNU_SOURCE

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/mman.h>

#include "proc.h"
#include "syscall.h"
#include "util.h"
#include "init.h"
#include "exn.h"

#include "hostexn.h"

#define SECTION(x) __attribute__((section(# x)))
#define ALIGN(x) __attribute__((aligned(x)))

#define TSECTION SECTION(".host.vm.text")
#define BSECTION SECTION(".host.vm.bss")

struct mmapent {
	void *from;
	unsigned sz;
	int prot;
};

struct kmmap {
	struct mmapent unlock_text;
	struct mmapent unlock_bss;
	struct mmapent procmaps[128];
	int n;
};

static struct kmmap BSECTION kmmap;
static char BSECTION stackbuf[0x5000];
static stack_t signal_stack = {
	.ss_sp = stackbuf,
	.ss_size = sizeof(stackbuf),
};

static bool syscall_hnd(int exn, struct context *c, void *arg) {
	ucontext_t *uc = (ucontext_t *) c;
	greg_t *regs = uc->uc_mcontext.gregs;

	if (0x81cd != *(uint16_t *) regs[REG_RIP]) {
		return false;
	}
	regs[REG_RIP] += 2;

	unsigned long ret = syscall_do(c, regs[REG_RAX],
			regs[REG_RBX], regs[REG_RCX],
			regs[REG_RDX], regs[REG_RSI],
			(void *) regs[REG_RDI]);
	regs[REG_RAX] = ret;
	return true;
}

static int TSECTION sysmprotect(void *addr, size_t size, int prot) {
	long ret;
	__asm__ __volatile__(
		"syscall\n"
		: "=a"(ret)
		: "a"(SYS_mprotect), "D"(addr), "S"(size), "d"(prot)
		: "memory"
	);
	return ret;
}

static void TSECTION host_vm_prot(bool restore) {
	void *load, *stack;
	int loadsz, stacksz;

	if (!restore) {
		struct proc *curp = current_process();
		load = curp->load;
		loadsz = curp->loadsz;
		stack = curp->stack;
		stacksz = curp->stacksz;
	}

	for (struct mmapent *e = kmmap.procmaps; e < kmmap.procmaps + ARRAY_SIZE(kmmap.procmaps); ++e) {
		sysmprotect(e->from, e->sz, restore ? e->prot : 0);
	}

	if (!restore) {
		sysmprotect(load,  loadsz,  PROT_READ | PROT_WRITE | PROT_EXEC);
		sysmprotect(stack, stacksz, PROT_READ | PROT_WRITE);
	}
}

static void TSECTION sighnd(int sig, siginfo_t *info, void *ctx) {
	host_vm_prot(true);
	exn_do(sig, (struct context *) ctx);
	host_vm_prot(false);
}

static int chprot2prot(char r, char w, char x) {
	assert(r == 'r' || r == '-');
	assert(w == 'w' || w == '-');
	assert(x == 'x' || x == '-');
	return (r == 'r' ? PROT_READ : 0) |
		(w == 'w' ? PROT_WRITE : 0) |
		(x == 'x' ? PROT_EXEC : 0);
}

static bool add_procmap(unsigned long from, unsigned long to, int prot) {
	if (ARRAY_SIZE(kmmap.procmaps) <= kmmap.n) {
		return false;
	}
	struct mmapent *e = &kmmap.procmaps[kmmap.n++];
	e->from = (void *) (uintptr_t) from;
	e->sz = to - from;
	e->prot = prot;
	return true;
}

static int host_vm_init(void) {
	extern char host_vm_text_start, host_vm_text_end, host_vm_bss_start, host_vm_bss_end;
	struct {
		unsigned long from;
		unsigned long to;
	} prothole[] = {
		{ (unsigned long) &host_vm_text_start, (unsigned long) &host_vm_text_end },
		{ (unsigned long) &host_vm_bss_start,  (unsigned long) &host_vm_bss_end },
		{ (unsigned long) kernel_globals.mem,  (unsigned long) ((char *) kernel_globals.mem + kernel_globals.memsz) },
	};

	bool ok = true;
	unsigned long from, to;
	char r, w, x;
	FILE *f = fopen("/proc/self/maps", "r");
	char name[64];
	name[0] = '\0';
	while (EOF != fscanf(f, "%lx-%lx %c%c%c%*c %*x %*d:%*d %*d%*[ \t]%63[^\n]\n", &from, &to, &r, &w, &x, name)) {
		if (!strcmp(name, "[stack]")) {
			continue;
		}
		if (strstr(name, "libpthread-")) {
			continue;
		}

		int prot = chprot2prot(r, w, x);
		bool intersects = false;
		for (int i = 0; i < ARRAY_SIZE(prothole); ++i) {
			unsigned long hs = prothole[i].from;
			unsigned long he = prothole[i].to;
			if (from <= hs && hs < to) {
				intersects = true;
				assert(he <= to);
				if (from < hs) {
					ok = ok & add_procmap(from, hs, prot);
				}
				if (he < to) {
					ok = ok & add_procmap(he, to, prot);
				}
			}
		}

		if (!intersects) {
			ok = ok & add_procmap(from, to, prot);
		}
		name[0] = '\0';
	}
	fclose(f);

	if (!ok) {
		kprint("%s: can't keep all proc lines\n", __func__);
	}
	return 0;
}

int exn_init(void) {
       int res;

       if ((res = host_vm_init())) {
	       return res;
       }

       if (sigaltstack(&signal_stack, NULL)) {
	       perror("sigaltstack");
	       return -1;
       }

       if ((res = exn_set_hnd(SIGSEGV, syscall_hnd, NULL))) {
	       return res;
       }

       struct sigaction act = {
               .sa_sigaction = sighnd,
               .sa_flags = SA_RESTART | SA_ONSTACK,
       };
       sigemptyset(&act.sa_mask);
       for (int i = 1; i < 32; ++i) {
               if (i == SIGKILL || i == SIGSTOP) {
                       continue;
               }
               if (-1 == sigaction(i, &act, NULL)) {
                       perror("irq init failed");
                       return -1;
               }
       }

       return 0;
}

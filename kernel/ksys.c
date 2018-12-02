#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "string.h"

#include "init.h"
#include "pool.h"
#include "palloc.h"
#include "time.h"
#include "kernel/util.h"
#include "hal/context.h"
#include "hal_context.h"
#include "hal/dbg.h"
#include "hal/exn.h"

#include "ksys.h"
#include "proc.h"

struct cpio_old_hdr {
	unsigned short   c_magic;
	unsigned short   c_dev;
	unsigned short   c_ino;
	unsigned short   c_mode;
	unsigned short   c_uid;
	unsigned short   c_gid;
	unsigned short   c_nlink;
	unsigned short   c_rdev;
	unsigned short   c_mtime[2];
	unsigned short   c_namesize;
	unsigned short   c_filesize[2];
};

#define EI_NIDENT 16
//               Name             Size Align Purpose
typedef uint64_t Elf64_Addr;   // 8    8     Unsigned program address
typedef uint64_t Elf64_Off;    // 8    8     Unsigned file offset
typedef uint16_t Elf64_Half;   // 2    2     Unsigned medium integer
typedef uint32_t Elf64_Word;   // 4    4     Unsigned integer
typedef  int32_t Elf64_Sword;  // 4    4     Signed integer
typedef uint64_t Elf64_Xword;  // 8    8     Unsigned long integer
typedef  int64_t Elf64_Sxword; // 8    8     Signed long integer

typedef struct {
        unsigned char   e_ident[EI_NIDENT];
        Elf64_Half      e_type;
        Elf64_Half      e_machine;
        Elf64_Word      e_version;
        Elf64_Addr      e_entry;
        Elf64_Off       e_phoff;
        Elf64_Off       e_shoff;
        Elf64_Word      e_flags;
        Elf64_Half      e_ehsize;
        Elf64_Half      e_phentsize;
        Elf64_Half      e_phnum;
        Elf64_Half      e_shentsize;
        Elf64_Half      e_shnum;
        Elf64_Half      e_shstrndx;
} Elf64_Ehdr;

#define PT_LOAD 1

typedef struct {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

static void *rootfs_cpio;

static struct proc procspace[8];
static struct pool procpool = POOL_INITIALIZER_ARRAY(procpool, procspace);

static struct proc *curp;
static TAILQ_HEAD(schedqueue, proc) squeue = TAILQ_HEAD_INITIALIZER(squeue);
static bool sched_posted;
static struct timer schedtimer;

struct sem {
	int cnt;
};

struct sem sems[1];
struct pool sempool = POOL_INITIALIZER_ARRAY(sempool, sems);

static const char *cpio_name(const struct cpio_old_hdr *ch) {
	return (const char*)ch + sizeof(struct cpio_old_hdr);
}

static const void *cpio_content(const struct cpio_old_hdr *ch) {
	return cpio_name(ch) + align(ch->c_namesize, 1);
}

static unsigned long cpio_filesize(const struct cpio_old_hdr *ch) {
	return (ch->c_filesize[0] << 16) | ch->c_filesize[1];
}

static const struct cpio_old_hdr *cpio_next(const struct cpio_old_hdr *ch) {
	return (const struct cpio_old_hdr *)
		((char*)cpio_content(ch) + align(cpio_filesize(ch), 1));
}

#if 0
static unsigned int cpio_rmajor(const struct cpio_old_hdr *ch) {
	return ch->c_rdev >> 8;
}

static unsigned int cpio_rminor(const struct cpio_old_hdr *ch) {
	return ch->c_rdev & 0xff;
}
#endif

int rootfs_cpio_init(void *p) {
	rootfs_cpio = p;
	return 0;
}

static const struct cpio_old_hdr *find_exe(char *name) {
	const struct cpio_old_hdr *start = rootfs_cpio;
	const struct cpio_old_hdr *found = NULL;

	const struct cpio_old_hdr *cph = start;
	while (strcmp(cpio_name(cph), "TRAILER!!!")) {
		if (!strcmp(cpio_name(cph), name)) {
			found = cph;
			break;
		}
		cph = cpio_next(cph);
	}
	return found;
}

static int load(char *name, void **entry, struct proc *proc) {
	const struct cpio_old_hdr *ch = find_exe(name);
	if (!ch) {
		return -1;
	}
	const char *rawelf = cpio_content(ch);

	const char elfhdrpat[] = { 0x7f, 'E', 'L', 'F', 2 };
	if (memcmp(rawelf, elfhdrpat, sizeof(elfhdrpat))) {
		return -1;
	}

	const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *) rawelf;
	if (!ehdr->e_phoff ||
			!ehdr->e_phnum ||
			!ehdr->e_entry ||
			ehdr->e_phentsize != sizeof(Elf64_Phdr)) {
		return -1;
	}
	const Elf64_Phdr *phdrs = (const Elf64_Phdr *) (rawelf + ehdr->e_phoff);

	const Elf64_Phdr *loadhdr = NULL;
	for (int i = 0; i < ehdr->e_phnum; ++i) {
		if (phdrs[i].p_type == PT_LOAD) {
			loadhdr = phdrs + i;
			break;
		}
	}
	if (!loadhdr) {
		return -1;
	}

	if (ehdr->e_entry < loadhdr->p_vaddr ||
			loadhdr->p_vaddr + loadhdr->p_memsz <= ehdr->e_entry) {
		return -1;
	}

	proc->loadn = psize(loadhdr->p_memsz);
	proc->load = palloc(proc->loadn);

	memset(proc->load, 0, loadhdr->p_memsz);
	memcpy(proc->load, rawelf + loadhdr->p_offset, loadhdr->p_filesz);

	*entry = proc->load + ehdr->e_entry - loadhdr->p_vaddr;
	return 0;
}

static void unload(struct proc *proc) {
	pfree(proc->load, proc->loadn);
}

struct proc *current_process() {
	return curp;
}

static int argv_size(char **argv, int *nargv) {
	int s = 0;
	char **a = argv;
	while (*a) {
		s += strlen(*a) + 1;
		++a;
	}
	*nargv = (a - argv) + 1;
	return s + *nargv * sizeof(char*);
}

static char **argv_copy(void *buf, size_t bufsz, char *argv[], int nargv) {
	char **ra = buf;
	char *p = buf + nargv * sizeof(char*);
	char **a = argv;
	while (*a) {
		*ra = p;
		strcpy(p, *a);
		p += strlen(*a) + 1;
		++ra;
		++a;
	}
	*ra = NULL;
	return (char **) buf;
}

static void sched_add(struct proc *new) {
	assert(!new->inqueue);
	TAILQ_INSERT_TAIL(&squeue, new, lentry);
	new->inqueue = true;
}

static void sched_remove(struct proc *p) {
	assert(p->inqueue);
	TAILQ_REMOVE(&squeue, p, lentry);
	p->inqueue = false;
}

static struct proc *sched_next(void) {
	return TAILQ_FIRST(&squeue);
}

static void sched_wake(struct proc *p) {
	bool irq = irq_save();
	if (!p->inqueue) {
		sched_add(p);
	}
	if (p->sleep) {
		p->sleep = false;
	}
	irq_restore(irq);
}

void sched(bool voluntary) {
	bool irq = irq_save();

	assert(!curp->inqueue);

	timer_stop(&schedtimer);

	if (!curp->sleep || !voluntary) {
		sched_add(curp);
	}

#if 0
	kprint("P %d %p %d %p ", curp - procspace, curp->stack, curp->stackn, curp->ctx.rsp);
	struct proc *ip;
	kprint("Q%d:", (int)voluntary);
	TAILQ_FOREACH(ip, &squeue, lentry) {
		kprint(" %d", ip - procspace);
	}
	kprint("\n");
#endif

	struct proc *nextp = sched_next();
	assert(nextp);
	sched_remove(nextp);

	timer_start(&schedtimer);

	if (curp != nextp) {
		struct proc *old = curp;
		struct proc *new = nextp;
		curp = nextp;
		ctx_switch(&old->ctx, &new->ctx);
	}

	irq_restore(irq);
}


static void preempt_sched(void *arg) {
	sched_posted = true;
}

void sched_handle_posted(void) {
	if (sched_posted) {
		sched_posted = false;
		sched(false);
	}
}

int sched_init(void) {
	struct proc *newp = pool_alloc(&procpool);
	if (!newp) {
		return -1;
	}
	newp->inqueue = false;
	newp->sleep = false;
	curp = newp;

	sched_posted = false;
	timer_init(&schedtimer, 100, false, preempt_sched, NULL);
	timer_start(&schedtimer);

	return 0;
}

void proctramp(void) {
	irq_restore(true);
	curp->entry();
}

int sys_run(char *argv[]) {
	if (!argv[0]) {
		return -1;
	}

	struct proc *newp = pool_alloc(&procpool);
	if (!newp) {
		goto failproc;
	}
	newp->parent = curp;
	newp->inqueue = false;
	newp->sleep = false;
	newp->exited = false;

	void *entry;
	if (load(argv[0], &entry, newp)) {
		goto failload;
	}

	newp->stackn = 2;
	newp->stack = palloc(newp->stackn);
	if (!newp->stack) {
		goto failstack;
	}

	int nargv;
	int asize = argv_size(argv, &nargv);
	newp->argvbn = psize(asize);
	newp->argvb = palloc(newp->argvbn);
	newp->argv = argv_copy(newp->argvb, asize, argv, nargv);
	newp->nargv = nargv;
	newp->entry = entry;

	ctx_make(&newp->ctx, proctramp, newp->stack, PSIZE * newp->stackn);
	bool irq = irq_save();
	sched_add(newp);
	irq_restore(irq);

	return newp - procspace;

failstack:
	unload(newp);
failload:
	pool_free(&procpool, newp);
failproc:
	return -1;
}

int sys_getargv(char *buf, int bufsz, char **argv, int argvsz) {
	int argc = 0;
	char *bufp = buf;

	for (char **arg = current_process()->argv; *arg; ++arg) {
		if (argvsz < argc) {
			return -1;
		}

		int len = strlen(*arg);
		if (buf + bufsz - bufp < len) {
			return -1;
		}

		strcpy(bufp, *arg);
		argv[argc++] = bufp;
		bufp += len + 1;
	}
	if (argvsz <= argc) {
		return -1;
	}
	argv[argc] = NULL;
	return argc;
}

int sys_exit(int code) {
	if (!curp->parent) {
		// init exits
		hal_halt();
	}

	curp->code = code;
	curp->exited = true;
	sched_wake(curp->parent);
	while (true) {
		curp->sleep = true;
		sched(true);
	}
	return 0;
}

int sys_wait(int id) {
	if (id < 0 || ARRAY_SIZE(procspace) <= id) {
		return -1;
	}

	struct proc *child = &procspace[id];
	curp->sleep = true;
	while (!child->exited) {
		sched(true);
		curp->sleep = true;
	}
	curp->sleep = false;

	assert(!child->inqueue && child->sleep);
	int code = child->code;
	pfree(child->argvb, child->argvbn);
	pfree(child->stack, child->stackn);
	unload(child);
	pool_free(&procpool, child);
	return code;
}

int sys_read(int f, void *buf, size_t sz) {
	return dbg_in(buf, sz);
}

int sys_write(int f, const void *buf, size_t sz) {
	dbg_out(buf, sz);
	return 0;
}

int sys_sem_alloc(int cnt) {
	bool irq = irq_save();
	struct sem *s = pool_alloc(&sempool);
	irq_restore(irq);

	if (!s) {
		return -1;
	}
	s->cnt = cnt;
	return s - sems;
}

int sys_sem_up(int id) {
	bool irq = irq_save();
	++sems[id].cnt;
	// TODO ping waiters
	irq_restore(irq);
	return -1;
}

int sys_sem_down(int id) {
	struct sem *s = &sems[id];
	bool irq = irq_save();
	if (s->cnt) {
		--s->cnt;
		goto out;
	}

	while (1) {
		irq_restore(irq);
		// TODO sleep
		sched(true);
		irq = irq_save();
		if (s->cnt) {
			--s->cnt;
			break;
		}
	}
out:
	irq_restore(irq);
	return 0;
}

int sys_sleep(int msec) {
	if (msec == 0) {
		sched(true);
		return 0;
	}

	int till = 1000 * msec + time_current();
	while (time_current() < till) {
		/*sched(true);*/
	}
	return 0;
}

int sys_uptime(void) {
	return time_current();
}

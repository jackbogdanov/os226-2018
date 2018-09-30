
#include <stddef.h>
#include <stdint.h>
#include "string.h"

#include "init.h"
#include "pool.h"
#include "kernel/util.h"
#include "hal/context.h"
#include "hal_context.h"
#include "hal/dbg.h"

#include "ksys.h"
#include "proc.h"
#include "exn.h"

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

static struct proc procspace[8];
static struct pool procpool = POOL_INITIALIZER_ARRAY(procpool, procspace);

static struct proc *curp;

static char *execbufp;
static void allocinit() {
	if (!execbufp) {
		execbufp = kernel_globals.mem;
	}
}

static void *alloc(int size) {
	const size_t execbufsz = kernel_globals.memsz;
	char *const execbuf = kernel_globals.mem;

	allocinit();

	if (execbuf + execbufsz - execbufp < size) {
		return NULL;
	}

	char *curexecbuf = execbufp;
	execbufp += size;
	return curexecbuf;
}

static void freeup(void *mark) {
	allocinit();
	execbufp = mark;
}

static void *freemark(void) {
	allocinit();
	return execbufp;
}

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

static const struct cpio_old_hdr *find_exe(char *name) {
	const struct cpio_old_hdr *start = kernel_globals.rootfs_cpio;
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

int load(char *name, void **entry, struct proc *proc) {
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

	const int loadsz = align(loadhdr->p_memsz, 0xfff);
	char *loadp = alloc(loadsz);
	memset(loadp, 0, loadhdr->p_memsz);
	memcpy(loadp, rawelf + loadhdr->p_offset, loadhdr->p_filesz);

	*entry = loadp + ehdr->e_entry - loadhdr->p_vaddr;
	proc->load = loadp;
	proc->loadsz = loadsz;
	return 0;
}

#if 0
static void tramprun(unsigned long *args) {
	int *code = (int *) args[0];
	char **argv = (char **) args[1];
	int (*entry)(int, char **) = (void *) args[2];
	void *mark = (void *) args[3];

	char **ap = argv + 1;
	while (*ap != NULL) {
		++ap;
	}

	int r = entry(ap - argv, argv);
	if (code) {
		*code = r;
	}

	freeup(mark);
}
#endif

struct proc *current_process() {
	return curp;
}

static bool syscall_hnd(int exn, struct context *c, void *arg) {
	struct proc * curp = current_process();
	ctx_save(c, &curp->savectx, arg, curp->stack, curp->stacksz);
	return true;
}

int run_first(char *argv[]) {
	struct proc *newp = pool_alloc(&procpool);
	assert(newp);

	newp->freemark = NULL;

	void *entry;
	if (load(argv[0], &entry, newp)) {
		goto failload;
	}

	newp->stacksz = 0x2000;
	newp->stack = alloc(newp->stacksz);
	if (!newp->stack) {
		goto failstack;
	}

	newp->argv = argv;
	newp->code = NULL;
	newp->prev = NULL;
	curp = newp;


	void (*mentry)(void) = entry;
	mentry();

	hal_halt();
	while (1);
failstack:
	freeup(newp->freemark);
failload:
	pool_free(&procpool, newp);
	return -1;
}

int sys_set_task(struct context *ctx, char *argv[]) {
	
	void *note = alloc(sizeof(char) * 50);
	memcpy(note, (void *) argv, sizeof(char) * 50);
	for(int i = 0; i < MAX_TASKS_IN_QUEUE; i++) {
		if (kernel_globals.tasks[i] == NULL){
			kernel_globals.tasks[i] = (char **) note;
			return 0;	
		}
	}

	return -1;
}

char ** find_task_for_run() {
	char **argv = NULL;
	int max = -1, id = -1;
	for(int i = 0; i < MAX_TASKS_IN_QUEUE; i++) {
		if (kernel_globals.tasks[i] != NULL) {
			int tprior = atoi(kernel_globals.tasks[i][0]);
			if (max < tprior && tprior != 0){
			max = tprior;
			argv = (kernel_globals.tasks[i] + 1);
			id = i;
			}
		}
	}
	
	if (argv) {
		//TO_DO FREE (imposible with during allocator)
		kernel_globals.tasks[id] = NULL;
	}
	return argv;
}

int sys_run(struct context *ctx, int *code) {
	char **argv = find_task_for_run(); 
	if (!argv) {
		return -2;
	}
	
	struct proc *newp = pool_alloc(&procpool);
	if (!newp) {
		goto failproc;
	}

	newp->freemark = freemark();

	void *entry;
	if (load(argv[0], &entry, newp)) {
		goto failload;
	}

	struct proc *oldp = curp;
	newp->stacksz = 0x2000;
	newp->stack = alloc(newp->stacksz);
	if (!newp->stack) {
		goto failstack;
	}

	newp->argv = argv;
	newp->code = code;
	ctx_save(ctx, &oldp->savectx, entry, newp->stack, newp->stacksz);
	newp->prev = oldp;
	curp = newp;

	return 0;
failstack:
	freeup(newp->freemark);
failload:
	pool_free(&procpool, newp);
failproc:
	return -1;
}

int sys_getargv(struct context *ctx, char *buf, int bufsz, char **argv, int argvsz) {
	int argc = 0;
	char *bufp = buf;

	for (char **arg = curp->argv; *arg; ++arg) {
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

int sys_exit(struct context *ctx, int code) {
	struct proc *newp = curp;
	struct proc *oldp = newp->prev;

	if (!oldp) {
		// init exits
		hal_halt();
	}

	if (newp->code) {
		*newp->code = code;
	}
	ctx_restore(ctx, &oldp->savectx);

	freeup(newp->freemark);
	pool_free(&procpool, newp);
	curp = oldp;
	return 0;
}

int sys_read(struct context *ctx, int f, void *buf, size_t sz) {
	return dbg_in(buf, sz);
}

int sys_write(struct context *ctx, int f, const void *buf, size_t sz) {
	dbg_out(buf, sz);
	return 0;
}

int sys_set_hnd(struct context *ctx, int sign, void * hnd) {
	exn_set_hnd(sign, syscall_hnd, hnd);
	return 0;
}

//1 echo 1; 2 echo 2; 3 echo 3; 4 echo 4;


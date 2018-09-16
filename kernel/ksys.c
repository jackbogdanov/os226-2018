
#include <stddef.h>
#include <stdint.h>
#include "string.h"

#include "init.h"
#include "kernel/util.h"
#include "hal/context.h"
#include "hal_context.h"
#include "hal/dbg.h"

#include "ksys.h"

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

#define ST_WORK 0
#define ADD_WORK 1

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

void * mem_alloc(size_t size, short flag) {

	if (flag == ST_WORK) {
		void * mark = kernel_globals.mem_ptr;
		kernel_globals.block_sizes[kernel_globals.block_ptr] = size;
		kernel_globals.block_ptr++;
		kernel_globals.mem_ptr += size;
		return 	mark;
	} else {
		kernel_globals.mem_ptr += size;
		kernel_globals.block_sizes[kernel_globals.block_ptr + CURRENT_SIZE_PTR_OFFSET] += size;
		return NULL;
	}

}

void free_mem(void *ptr) {
	size_t block_size = kernel_globals.block_sizes[kernel_globals.block_ptr + PREVIOUS_SIZE_PTR_OFFSET];
	kernel_globals.mem_ptr -= block_size;
	kernel_globals.block_ptr--;
}

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

void *load(char *name, void **entry) {
	const struct cpio_old_hdr *ch = find_exe(name);
	if (!ch) {
		return NULL;
	}

	const char *rawelf = cpio_content(ch);
	Elf64_Ehdr *elf_ptr = (Elf64_Ehdr *) rawelf;
	Elf64_Phdr *sec_table = (Elf64_Phdr *) (rawelf + elf_ptr->e_phoff);
	void *mark = mem_alloc(sec_table->p_filesz, ST_WORK);

	int sec_num = elf_ptr->e_phnum;

	while(sec_num--) {
		if (sec_table->p_type == PT_LOAD) {
			memcpy(mark + sec_table->p_vaddr, rawelf + sec_table->p_offset, sec_table->p_filesz);
			if (sec_num) {
				mem_alloc(sec_table->p_filesz, ADD_WORK);
			}
		}
		sec_table = (Elf64_Phdr *) ((unsigned char *) sec_table + elf_ptr->e_shentsize);
	}

	*entry = mark + elf_ptr->e_entry;
	return mark;
}

void unload(void *mark) {
	free_mem(mark);
}

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

	unload(mark);
}

int sys_run(struct context *ctx, char *argv[], int *code) {
	if (!argv[0]) {
		return -1;
	}

	void *entry;
	void *mark = load(argv[0], &entry);
	if (!mark) {
		return -1;
	}
	struct context_call_save save;

	ctx_call_setup(ctx, tramprun, &save);
	ctx_push(ctx, (unsigned long) mark);
	ctx_push(ctx, (unsigned long) entry);
	ctx_push(ctx, (unsigned long) argv);
	ctx_push(ctx, (unsigned long) code);
	ctx_call_end(ctx, &save);

	return 0;
}

int sys_read(struct context *ctx, int f, void *buf, size_t sz) {
	return dbg_in(buf, sz);
}

int sys_write(struct context *ctx, int f, const void *buf, size_t sz) {
	dbg_out(buf, sz);
	return 0;
}


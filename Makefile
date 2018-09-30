MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

.PHONY : all clean force force-rootfs

all:

CC = gcc
LD = ld
HAL := host

CFLAGS ?= -std=c99 -Wall -Werror
CFLAGS += -MMD -MT $@ -MF $@.d

DEBUG=1
ifneq ($(DEBUG),)
CFLAGS += -g
LDFLAGS += -g
endif

PCFLAGS = -ffreestanding -fno-stack-protector -fpie --sysroot=./sysroot
UCFLAGS = $(PCFLAGS) -I=/include

HALDIR = ./$(HAL)hal

HCLFAGS = -iquote $(HALDIR)/include

BUILD = ./build
IMAGE = $(BUILD)/image
ROOTFS = $(BUILD)/rootfs.cpio

LIBC = $(BUILD)/libc.a

KSRC = $(wildcard kernel/*.c)
KOBJ = $(KSRC:%.c=$(BUILD)/%.o)
$(KOBJ) : CFLAGS += $(PCFLAGS) $(HCLFAGS) -iquote =/include #-iquote ./kernel

HCSRC = $(wildcard $(HALDIR)/*.c)
HSSRC = $(wildcard $(HALDIR)/*.S)
HOBJ = $(HCSRC:%.c=$(BUILD)/%.o) $(HSSRC:%.S=$(BUILD)/%.o)
$(HOBJ) : CFLAGS += $(HCLFAGS) -iquote ./kernel

CSRC = $(wildcard libc/*.c)
COBJ = $(CSRC:%.c=$(BUILD)/%.o)
$(COBJ) : CFLAGS += $(UCFLAGS)

ASRC = $(wildcard apps/*.c)
AOBJ = $(ASRC:%.c=$(BUILD)/%.o)
AEXE = $(AOBJ:%.o=%)
$(AOBJ) : CFLAGS += $(UCFLAGS) -fPIC

$(AOBJ:%.o=%) : $(LIBC)
$(AEXE) : % : %.o
	ld --pic-executable -nostdlib -N -e _main -o $@ $< $(LIBC)

all : $(IMAGE) $(ROOTFS)

$(IMAGE) : $(KOBJ) $(HOBJ) $(LIBC)
	$(CC) $(LDFLAGS) -o $@ $^ -lrt -lpthread $(HALDIR)/kernel.ld

$(BUILD)/%.o : %.c
	@mkdir -p $(@D)
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD)/%.o : %.S
	@mkdir -p $(@D)
	$(CC) -c $(ASFLAGS) -o $@ $<

$(LIBC) : $(COBJ)
	ar rcs $@ $^

$(BUILD)/%.app.o : $(BUILD)/%.o $(LIBC)
	ld -T ./app.ld -r -o $@.tmp $< $(LIBC)
	ld -o $@ $@.tmp

$(ROOTFS) : $(AEXE)
	(cd $(BUILD)/apps; find -type f -not -name '*.*' | cpio -H bin -o) > $@

init-repo :
	sudo mknod rootfs/dbg c 4 1

clean :
	rm -rf ./build


-include $(patsubst %,%.d,$(KOBJ) $(KOBJ) $(HOBJ) $(COBJ) $(AOBJ))

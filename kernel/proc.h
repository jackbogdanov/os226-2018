#pragma once

#include "hal_context.h"

struct proc {
	struct proc *prev;
	struct context savectx;

	void *freemark;

	void *load;
	int loadsz;
	void *stack;
	int stacksz;

	char **argv;
	int *code;
};

extern struct proc *current_process();

#pragma once

#include <stdbool.h>
#include "hal_context.h"
#include "third-party/queue.h"

struct proc {
	TAILQ_ENTRY(proc) lentry;
	struct uctx ctx;
	struct proc *parent;

	int code;

	bool sleep;
	bool inqueue;
	bool exited;

	void *load;
	int loadn;

	void *stack;
	int stackn;

	void *argvb;
	int argvbn;
	char **argv;
	int nargv;
	void (*entry)(void);
};

extern struct proc *current_process();

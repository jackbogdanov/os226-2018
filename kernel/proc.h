#pragma once

#include "hal_context.h"
#include "third-party/queue.h"

struct proc {
	TAILQ_ENTRY(proc) lentry;
	struct uctx ctx;
	struct proc *parent;
	int code;
	enum state {
		EXITED,
		SLEEPING,
		READY,
		RUNNING,
	} state;

	void *freemark;
	void *load;
	int loadsz;
	void *stack;
	int stacksz;

	char *argvb;
	char **argv;
	int nargv;
};

extern struct proc *current_process();

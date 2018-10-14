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
		SLEEP_TRANSITION,
		READY,
		RUNNING,
	} state;

	void *load;
	int loadn;

	void *stack;
	int stackn;

	void *argvb;
	int argvbn;
	char **argv;
	int nargv;
};

extern struct proc *current_process();

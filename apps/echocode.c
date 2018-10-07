
#include <stdio.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		rprintf("Not enough arguments\n");
		return 1;
	}

	int p = os_run(argv + 1);
	if (p < 0) {
		rprintf("Failed to run");
		return 1;
	}
	int code = os_wait(p);

	rprintf("Exit code: %d\n", code);
	return 0;
}

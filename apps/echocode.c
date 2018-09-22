
#include <stdio.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		rprintf("Not enough arguments\n");
		return 1;
	}

	int code = -1;
	if (os_run(argv + 1, &code)) {
		rprintf("Failed to run");
		return 1;
	}

	rprintf("Exit code: %d\n", code);
	return 0;
}

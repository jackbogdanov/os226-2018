#include <stdio.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	
	rprintf("BEFORE FORK\n");
        rprintf(argv[0]);

	os_fork();
	for (int i = 0; i < 50; ++i) {
		rprintf("%d\n", i);
	}

	rprintf(argv[0]);

	return 0;
}

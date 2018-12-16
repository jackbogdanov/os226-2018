#include <stdio.h>
#include <kernel/syscall.h>
int main(int argc, char *argv[]) {
	rprintf("BEFORE\n");

	int res = os_fork();
	for (int i = 0; i < 50; ++i) {
		rprintf("%d\n", i);
	}
	rprintf("fork results %d\n", res);

	rprintf("end\n");
	return 0;
}

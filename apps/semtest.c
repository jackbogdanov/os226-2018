#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	long id = strtol(argv[1], NULL, 10);
	if (1 < id) {
		char c[8];
		snprintf(c, sizeof(c), "%d", id - 1);
		rprintf("%s %s\n", argv[0], c);
		char *newargv[] = { argv[0], c, NULL };
		int r = os_run(newargv);
		if (r < 0) {
			rprintf("cant create new: %d\n", r);
		}
	}

	while (true) {
		os_sem_down(0);
		rprintf("in  %d\n", id);
		os_sleep(0);
		os_sleep(0);
		os_sleep(0);
		rprintf("out %d\n", id);
		os_sem_up(0);
	}

	return 0;
}

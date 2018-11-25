#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	int sem = (2 < argc) ? strtol(argv[2], NULL, 10) : os_sem_alloc(1);
	long id = strtol(argv[1], NULL, 10);
	if (1 < id) {
		char c[8];
		snprintf(c, sizeof(c), "%d", id - 1);
		char csem[8];
		snprintf(csem, sizeof(csem), "%d", sem);
		char *newargv[] = { argv[0], c, csem, NULL };
		int r = os_run(newargv);
		if (r < 0) {
			rprintf("cant create new: %d\n", r);
		}
	}

	rprintf("%s id %d semid %d\n", argv[0], id, sem);
	while (true) {
		os_sem_down(sem);
		rprintf("in  %d\n", id);
		os_sleep(200);
		rprintf("out %d\n", id);
		os_sem_up(sem);
		os_sleep(200);
	}

	return 0;
}

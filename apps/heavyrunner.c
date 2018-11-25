#include <stddef.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	int some;
	for (int i = 1; i <= argc - 1; ++i) {
		char *cargv[] = { "heavy", argv[i], NULL };
		some = os_run(cargv);
	}
	os_wait(some);
	return 0;
}


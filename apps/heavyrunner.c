#include <stddef.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	for (int i = 1; i <= argc - 1; ++i) {
		char *cargv[] = { "heavy", argv[i], NULL };
		os_run(cargv);
	}
	while(1);
}


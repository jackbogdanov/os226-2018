
#include <string.h>
#include <kernel/syscall.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

extern int main(int argc, char *argv[]);

void _main(void) {
	char args[128];
	char *argv[16];
	int argc = os_getargv(args, ARRAY_SIZE(args), argv, ARRAY_SIZE(argv));
	if (argc < 0) {
		os_exit(256);
	}

	int c = main(argc, argv);
	os_exit(c);
}

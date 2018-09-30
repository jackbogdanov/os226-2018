#include <string.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	int sec = strtol(argv[1], NULL, 10);
	os_sleep(sec * 1000);
	return 0;
}

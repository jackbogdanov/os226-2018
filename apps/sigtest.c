
#include <stdio.h>
#include <kernel/syscall.h>

volatile int *badaddr = 0x0;
static int hndnest = 0;

void dosig(void) {
	*badaddr = 1;
}

void hnd(int sig) {
	++hndnest;
	rprintf("signal nest %d\n", hndnest);
	if (1 < hndnest) {
		os_exit(0);
	}
	dosig();
	os_exit(1);
}

int main(int argc, char *argv[]) {
	dosig();
	rprintf("missed signal\n");
	return 1;
}

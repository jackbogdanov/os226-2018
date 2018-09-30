#include <stdio.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	const int base = 1000000;
	int uptime = os_uptime();
	rprintf("%d.%d\n", uptime / base, uptime % base);
	return 0;
}



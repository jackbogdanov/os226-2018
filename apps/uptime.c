#include <stdio.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	int uptime = os_uptime();
	rprintf("%d\n", uptime);
	return 0;
}



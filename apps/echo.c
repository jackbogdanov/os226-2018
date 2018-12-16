
#include <stdio.h>

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; ++i) {
		rprintf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
	}
	return 0;
}

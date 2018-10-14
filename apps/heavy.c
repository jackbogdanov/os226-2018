
#include <stdbool.h>
#include <stdio.h>

int main(int argc, char *argv[0]) {
	if (1 < argc) {
		while (true) {
			rprintf("%s\n", argv[1]);
		}
	} else {
		while(true);
	}
	return 0;
}

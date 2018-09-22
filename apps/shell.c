
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <kernel/syscall.h>

int main(int argc, char *argv[]) {
	while (1) {
		rprintf("> ");
		char buffer[256];
		int bytes = os_read(0, buffer, sizeof(buffer));
		if (!bytes) {
			break;
		}

		if (bytes < sizeof(buffer)) {
			buffer[bytes] = '\0';
		}

		const char *comsep = "\n;";
		char *stcmd;
		char *cmd = strtok_r(buffer, comsep, &stcmd);
		while (cmd) {
			const char *argsep = " ";
			char *starg;
			char *arg = strtok_r(cmd, argsep, &starg);
			char *argv[256];
			int argc = 0;
			while (arg) {
				argv[argc++] = arg;
				arg = strtok_r(NULL, argsep, &starg);
			}
			argv[argc] = NULL;

			if (!argc) {
				break;
			}

			os_run(argv, NULL);
			cmd = strtok_r(NULL, comsep, &stcmd);
		}
	}

	rprintf("\n");
	return 0;
}



#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kernel/syscall.h>

int rprintf(const char *format, ...) {
	char msg[128];
	va_list ap;
	va_start(ap, format);
	int ret = vsnprintf(msg, sizeof(msg), format, ap);
	va_end(ap);
	return os_write(1, msg, ret);
}

int snprintf(char *str, size_t size, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int ret = vsnprintf(str, size, format, ap);
	va_end(ap);
	return ret;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
	bool spec = false;
	char *p = str;
	char c;
	while ((c = *format++)) {
		char b[16];
		char *o = NULL;
		size_t os = 0;

		if (!spec) {
			if (c != '%') {
				b[0] = c;
				o = b;
				os = 1;
			} else {
				spec = true;
			}
		} else {
			switch (c) {
			case 's':
				o = va_arg(ap, char *);
				os = strlen(o);
				break;
			case 'd': {
				char *p = b + sizeof(b) - 1;
				int d = va_arg(ap, int);
				if (!d) {
					*p-- = '0';
				} else {
					bool m = d < 0;
					if (m) {
						d = -d;
					}
					while (d && p != b) {
						*p-- = '0' + d % 10;
						d /= 10;
					}
					if (p != b && m) {
						*p-- = '-';
					}
				}
				o = p + 1;
				os = b + sizeof(b) - o;
				break;
			}
			case 'c':
				b[0] = (char) va_arg(ap, int);
				o = b;
				os = 1;
				break;
			default:
				b[0] = c;
				o = b;
				os = 1;
				break;
			}
			spec = false;
		}

		if (os < str + size - p - 1) {
			memcpy(p, o, os);
			p += os;
		} else {
			break;
		}
	}
	*p = '\0';
	va_end(ap);
	return p - str;
}

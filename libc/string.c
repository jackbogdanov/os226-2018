
#include <stddef.h>
#include <string.h>

int strcmp(const char *s1, const char *s2) {
	while (*s1 && *s1 == *s2) {
		++s1;
		++s2;
	}
	return *s1 - *s2;
}

size_t strlen(const char *s) {
	const char *p = s;
	while (*p) {
		++p;
	}
	return p - s;
}

char *strcpy(char *dest, const char *src) {
	char *p = dest;
	while (*src) {
		*dest++ = *src++;
	}
	return p;
}

char *strchr(const char *s, int c) {
	for (const char *p = s; *p != '\0'; ++p) {
		if (*p == c) {
			return (char *) p;
		}
	}
	return NULL;
}

size_t strspn(const char *str, const char *accept) {
	const char *s = str;
	while (*s != '\0'  && strchr(accept, *s)) {
		++s;
	}
	return s - str;
}


size_t strcspn(const char *str, const char *reject) {
	const char *s = str;
	while (*s != '\0'  && !strchr(reject, *s)) {
		++s;
	}
	return s - str;
}

char *strtok_r(char *str, const char *delim, char **saveptr) {
	char *s = str ? str : *saveptr;
	s += strspn(s, delim);
	if (*s == '\0') {
		*saveptr = s;
		return NULL;
	}

	char *e = s + strcspn(s, delim);
	if (*e == '\0') {
		*saveptr = e;
	} else {
		*e = '\0';
		*saveptr = e + 1;
	}
	return s;
}

void *memset(void *s, int c, size_t n) {
	char *e = (char *)s + n;
	for (char *p = s; p < e; ++p) {
		*p = c;
	}
	return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
	const char *e = src + n;
	const char *s = src;
	char *d = dest;
	while (s < e) {
		*d++ = *s++;
	}
	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const char *ss1 = s1;
	const char *ss2 = s2;
	for (int i = 0; i < n; ++i) {
		if (ss1[i] != ss2[i]) {
			return ss1[i] - ss2[i];
		}
	}
	return 0;
}

long int strtol(const char *nptr, char **endptr, int base) {
	// IMPL ME
	return 0;
}

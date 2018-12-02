#pragma once

#include <stddef.h>

int strcmp(const char *s1, const char *s2);

size_t strlen(const char *s);

char *strcpy(char *dest, const char *src);

char *strtok_r(char *str, const char *delim, char **saveptr);

char *strchr(const char *s, int c);

size_t strspn(const char *str, const char *accept);

size_t strcspn(const char *str, const char *reject);

void *memset(void *s, int c, size_t n);

void *memcpy(void *dest, const void *src, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);

long int strtol(const char *nptr, char **endptr, int base);


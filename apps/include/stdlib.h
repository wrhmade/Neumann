#ifndef STDLIB_H
#define STDLIB_H
#include <stddef.h>
#include <stdint.h>
void *malloc(uint32_t size);
void free(void *buf);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t new_size);

double strtod(const char *str, char **endptr);
long double strtold(const char *str, char **endptr);

int rand(void);
void srand(unsigned int seed);
#endif
#ifndef STDIO_H
#define STDIO_H
#include <stdarg.h>
int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list ap);
int sprintf(char *s, const char *format, ...);
int vsprintf(char *buff, const char *format, va_list args);
void putchar(int c);
void puts(const char *buf);
int vsscanf(const char *str, const char *format, va_list ap);
int sscanf(char *s, const char *format, ...);
int vscanf(const char *format, va_list ap);
int scanf(const char *format, ...);
int getchar();
#endif
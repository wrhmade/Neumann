/*
stdio.h 
标准输入输出 
Copyright W24 Studio 
*/

#ifndef STDIO_H
#define STDIO_H
#include <stdarg.h>
int sprintf(char *s, const char *format, ...);
int vsprintf(char *buff, const char *format, va_list args);
#endif

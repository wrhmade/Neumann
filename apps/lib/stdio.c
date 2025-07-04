#include <stdio.h>
#include <stdarg.h>
#include <napi.h>
#include <unistd.h>


void puts(const char *buf)
{
    napi_putstr(buf);
}

int vprintf(const char *fmt, va_list ap)
{
    char buf[1024] = {0}; // 理论上够了
    int ret = vsprintf(buf, fmt, ap);
    napi_putstr(buf);
    return ret;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = vprintf(fmt, ap);
    va_end(ap);
    return ret;
}

void putchar(int c)
{
    char s[2];
    s[0]=(char)c;
    s[1]=0;
    napi_putstr(s);
}
#ifndef STDARG_H
#define STDARG_H

typedef char *va_list; // 我也不知道va_list是什么类型，先给个char *挂着，反正用不到

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)
#define va_copy(d,s)    __builtin_va_copy(d,s)

#endif

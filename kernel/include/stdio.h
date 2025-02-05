/*
stdio.h 
标准输入输出 
Copyright W24 Studio 
*/

#ifndef STDIO_H
#define STDIO_H
#include <stdarg.h>
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */
#ifdef __USE_GNU
# define SEEK_DATA	3	/* Seek to next data.  */
# define SEEK_HOLE	4	/* Seek to next hole.  */
#endif

int sprintf(char *s, const char *format, ...);
int vsprintf(char *buff, const char *format, va_list args);
#endif

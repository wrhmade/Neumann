#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <napi/consio/consi.h>
#include <napi/consio/conso.h>
#include <stdlib.h>

void puts(const char *buf)
{
    napi_puts(buf);
}

int vprintf(const char *fmt, va_list ap)
{
    char buf[1024] = {0}; // 理论上够了
    int ret = vsprintf(buf, fmt, ap);
    napi_puts(buf);
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
    napi_putc(c);
}

int sprintf(char *s, const char *format, ...)
{
    va_list args;
    int len;
 
    va_start(args, format);
    len = vsprintf(s,format,args);
    va_end(args);
 
    return len;
}

#define ZEROPAD		1	// pad with zero
#define SIGN	 	2   // unsigned/signed long
#define PLUS    	4	// show plus
#define SPACE	  	8   // space if plus
#define LEFT	 	16  // left justified
#define SPECIAL		32  // 0x
#define SMALL	  	64  // use 'abcdef' instead of 'ABCDEF'

#define do_div(n,base) ({ \
		int __res; \
		__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
		__res; })

static char *number(char *str, int num, int base, int size, int precision, int type)
{
	char c, sign, tmp[36];
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type & SMALL) {
		digits ="0123456789abcdefghijklmnopqrstuvwxyz";
	}
	if (type & LEFT) {
		type &= ~ZEROPAD;
	}
	if (base < 2 || base > 36) {
		return 0;
	}

	c = (type & ZEROPAD) ? '0' : ' ' ;

	if (type & SIGN && num < 0) {
		sign = '-';
		num = -num;
	} else {
		sign = (type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
	}

	if (sign) {
		size--;
	}
	if (type & SPECIAL) {
		if (base == 16) {
			size -= 2;
		} else if (base == 8) {
			size--;
		}
	}
	i = 0;
	if (num == 0) {
		tmp[i++] = '0';
	} else {
		while (num != 0) {
			tmp[i++] = digits[do_div(num,base)];
		}
	}

	if (i > precision) {
		precision = i;
	}
	size -= precision;

	if (!(type&(ZEROPAD+LEFT))) {
		while (size-- > 0) {
			*str++ = ' ';
		}
	}
	if (sign) {
		*str++ = sign;
	}
	if (type & SPECIAL) {
		if (base == 8) {
			*str++ = '0';
		} else if (base == 16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if (!(type&LEFT)) {
		while (size-- > 0) {
			*str++ = c;
		}
	}
	while (i < precision--) {
		*str++ = '0';
	}
	while (i-- > 0) {
		*str++ = tmp[i];
	}
	while (size-- > 0) {
		*str++ = ' ';
	}

	return str;
}

#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i = 0;

	while (is_digit(**s)) {
		i = i * 10 + *((*s)++) - '0';
	}

	return i;
}

int vsprintf(char *buff, const char *format, va_list args)
{
    int len;
    int i;
    char *str;
    char *s;
    int *ip;

    int flags;            // flags to number()

    int field_width;    // width of output field
    int precision;        // min. # of digits for integers; max number of chars for from string

    for (str = buff ; *format ; ++format) {
        if (*format != '%') {
            *str++ = *format;
            continue;
        }

        flags = 0;
        repeat:
            ++format;    // this also skips first '%'
        switch (*format) {
                case '-': flags |= LEFT;
                    goto repeat;
                case '+': flags |= PLUS;
                    goto repeat;
                case ' ': flags |= SPACE;
                    goto repeat;
                case '#': flags |= SPECIAL;
                    goto repeat;
                case '0': flags |= ZEROPAD;
                    goto repeat;
            }
        
        /* get field width */
        field_width = -1;
        if (is_digit(*format)) {
            field_width = skip_atoi(&format);
        } else if (*format == '*') {
            /* it's the next argument */
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*format == '.') {
            ++format;    
            if (is_digit(*format)) {
                precision = skip_atoi(&format);
            } else if (*format == '*') {
                /* it's the next argument */
                precision = va_arg(args, int);
            }
            if (precision < 0) {
                precision = 0;
            }
        }

        /* get the conversion qualifier */
        /* int qualifier = -1;    // 'h', 'l', or 'L' for integer fields */
        if (*format == 'h' || *format == 'l' || *format == 'L') {
            // qualifier = *format;
            ++format;
        }

        switch (*format) {
        case 'c':
            if (!(flags & LEFT)) {
                while (--field_width > 0) {
                    *str++ = ' ';
                }
            }
            *str++ = (unsigned char) va_arg(args, int);
            while (--field_width > 0) {
                *str++ = ' ';
            }
            break;

        case 's':
            s = va_arg(args, char *);
            len = strlen(s);
            if (precision < 0) {
                precision = len;
            } else if (len > precision) {
                len = precision;
            }

            if (!(flags & LEFT)) {
                while (len < field_width--) {
                    *str++ = ' ';
                }
            }
            for (i = 0; i < len; ++i) {
                *str++ = *s++;
            }
            while (len < field_width--) {
                *str++ = ' ';
            }
            break;

        case 'o':
            str = number(str, va_arg(args, unsigned long), 8,
                field_width, precision, flags);
            break;

        case 'p':
            if (field_width == -1) {
                field_width = 8;
                flags |= ZEROPAD;
            }
            str = number(str, (unsigned long) va_arg(args, void *), 16,
                field_width, precision, flags);
            break;

        case 'x':
            flags |= SMALL;
        case 'X':
            str = number(str, va_arg(args, unsigned long), 16,
                field_width, precision, flags);
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            str = number(str, va_arg(args, unsigned long), 10,
                field_width, precision, flags);
            break;
        case 'b':
            str = number(str, va_arg(args, unsigned long), 2,
                field_width, precision, flags);
            break;

        case 'n':
            ip = va_arg(args, int *);
            *ip = (str - buff);
            break;

        default:
            if (*format != '%')
                *str++ = '%';
            if (*format) {
                *str++ = *format;
            } else {
                --format;
            }
            break;
        }
    }
    *str = '\0';

    return (str -buff);
}


// 解析格式说明符的结构体
typedef struct {
    int width;          // 宽度指定
    int suppress;       // 是否抑制赋值（*）
    char length;        // 长度修饰符（h, l, L）
    char specifier;     // 转换说明符
    char *scanset;      // 扫描集字符串
    int scanset_invert; // 扫描集是否取反
} format_specifier;

// 解析格式字符串中的说明符
int parse_format_specifier(const char **format, format_specifier *spec) {
    const char *p = *format;
    
    // 初始化
    spec->width = 0;
    spec->suppress = 0;
    spec->length = '\0';
    spec->specifier = '\0';
    spec->scanset = NULL;
    spec->scanset_invert = 0;
    
    // 检查是否有%
    if (*p != '%') return 0;
    p++;
    
    // 检查抑制赋值*
    if (*p == '*') {
        spec->suppress = 1;
        p++;
    }
    
    // 解析宽度
    if (isdigit(*p)) {
        spec->width = 0;
        while (isdigit(*p)) {
            spec->width = spec->width * 10 + (*p - '0');
            p++;
        }
    }
    
    // 解析长度修饰符
    if (*p == 'h' || *p == 'l' || *p == 'L') {
        spec->length = *p;
        p++;
        // 处理ll的情况
        if (spec->length == 'l' && *p == 'l') {
            spec->length = 'L';  // 用L表示ll
            p++;
        }
    }
    
    // 检查扫描集
    if (*p == '[') {
        p++;
        spec->specifier = '[';
        
        // 检查是否取反
        if (*p == '^') {
            spec->scanset_invert = 1;
            p++;
        }
        
        // 分配内存并复制扫描集
        const char *start = p;
        while (*p && *p != ']') {
            p++;
        }
        
        if (*p == ']') {
            int len = p - start;
            spec->scanset = malloc(len + 1);
            strncpy(spec->scanset, start, len);
            spec->scanset[len] = '\0';
            p++;
        } else {
            // 无效的扫描集格式
            return 0;
        }
        
        *format = p;
        return 1;
    }
    
    // 获取转换说明符
    if (strchr("diouxXfFeEgGaAcspn%", *p)) {
        spec->specifier = *p;
        p++;
        *format = p;
        return 1;
    }
    
    return 0;
}

// 释放格式说明符资源
void free_format_specifier(format_specifier *spec) {
    if (spec->scanset) {
        free(spec->scanset);
        spec->scanset = NULL;
    }
}

// 跳过输入字符串中的空白字符
void skip_whitespace(const char **str) {
    while (**str && isspace(**str)) {
        (*str)++;
    }
}

// 检查字符是否在扫描集中
int is_in_scanset(char c, const char *scanset, int invert) {
    if (!scanset) return 0;
    
    // 检查字符是否在扫描集中
    int found = 0;
    const char *p = scanset;
    
    while (*p) {
        if (*p == '-' && p > scanset && *(p + 1)) {
            // 处理范围表示法 [a-z]
            char start = *(p - 1);
            char end = *(p + 1);
            if (start <= end && c >= start && c <= end) {
                found = 1;
                break;
            }
            p += 2;
        } else {
            if (c == *p) {
                found = 1;
                break;
            }
            p++;
        }
    }
    
    return invert ? !found : found;
}

// 从字符串中读取扫描集
int read_scanset(const char **str, int width, const char *scanset, int invert, 
                 va_list ap, int suppress) {
    const char *start = *str;
    int count = 0;
    
    // 读取匹配扫描集的字符
    while (**str && (width == 0 || count < width)) {
        if (is_in_scanset(**str, scanset, invert)) {
            (*str)++;
            count++;
        } else {
            break;
        }
    }
    
    if (count > 0 && !suppress) {
        char *val = va_arg(ap, char*);
        strncpy(val, start, count);
        val[count] = '\0';
        return 1;
    }
    
    return count > 0 ? 0 : -1;
}

// 从字符串中读取整数
int read_integer(const char **str, int width, char length, int base, va_list ap, int suppress) {
    long long value = 0;
    
    // 处理符号
    int negative = 0;
    if (**str == '-') {
        negative = 1;
        (*str)++;
        if (width > 0) width--;
    } else if (**str == '+') {
        (*str)++;
        if (width > 0) width--;
    }
    
    // 处理0x, 0X前缀
    if (base == 16 || base == 0) {
        if (**str == '0' && (*(*str + 1) == 'x' || *(*str + 1) == 'X')) {
            if (base == 0) base = 16;
            (*str) += 2;
            if (width > 0) width -= 2;
        }
    }
    
    if (base == 0) base = 10;
    
    // 读取数字
    int count = 0;
    while (**str && (width == 0 || count < width)) {
        char c = **str;
        int digit = -1;
        
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (base > 10) {
            if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        }
        
        if (digit < 0 || digit >= base) break;
        
        value = value * base + digit;
        (*str)++;
        count++;
    }
    
    if (negative) value = -value;
    
    // 存储结果（如果不抑制赋值）
    if (!suppress && count > 0) {
        if (length == 'h') {
            short *val = va_arg(ap, short*);
            *val = (short)value;
        } else if (length == 'l') {
            long *val = va_arg(ap, long*);
            *val = (long)value;
        } else if (length == 'L') {
            long long *val = va_arg(ap, long long*);
            *val = value;
        } else {
            int *val = va_arg(ap, int*);
            *val = (int)value;
        }
        return 1;
    }
    
    return count > 0 ? 0 : -1;
}

// 从字符串中读取指针地址
int read_pointer(const char **str, int width, va_list ap, int suppress) {
    
    // 检查0x前缀
    if (**str == '0' && (*(*str + 1) == 'x' || *(*str + 1) == 'X')) {
        (*str) += 2;
        if (width > 0) width -= 2;
    }
    
    // 读取十六进制数字
    int count = 0;
    uintptr_t value = 0;
    
    while (**str && (width == 0 || count < width)) {
        char c = **str;
        int digit = -1;
        
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else break;
        
        value = value * 16 + digit;
        (*str)++;
        count++;
    }
    
    if (!suppress && count > 0) {
        void **val = va_arg(ap, void**);
        *val = (void*)value;
        return 1;
    }
    
    return count > 0 ? 0 : -1;
}

// 从字符串中读取浮点数
int read_float(const char **str, int width, char length, va_list ap, int suppress) {
    char buffer[256];
    int count = 0;
    
    // 读取符号
    if (**str == '-' || **str == '+') {
        if (width > 0 && count < width) {
            buffer[count++] = **str;
            (*str)++;
        }
    }
    
    // 读取数字部分
    while (**str && (width == 0 || count < width)) {
        char c = **str;
        if (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+') {
            buffer[count++] = c;
            (*str)++;
        } else {
            break;
        }
    }
    
    buffer[count] = '\0';
    
    if (count > 0 && !suppress) {
        if (length == 'L') {
            long double *val = va_arg(ap, long double*);
            *val = strtold(buffer, NULL);
        } else {
            double *val = va_arg(ap, double*);
            *val = strtod(buffer, NULL);
        }
        return 1;
    }
    
    return count > 0 ? 0 : -1;
}

// 从字符串中读取字符串
int read_string(const char **str, int width, va_list ap, int suppress) {
    skip_whitespace(str);
    
    const char *start = *str;
    int count = 0;
    
    // 读取非空白字符
    while (**str && !isspace(**str) && (width == 0 || count < width)) {
        (*str)++;
        count++;
    }
    
    if (count > 0 && !suppress) {
        char *val = va_arg(ap, char*);
        strncpy(val, start, count);
        val[count] = '\0';
        return 1;
    }
    
    return count > 0 ? 0 : -1;
}

// 从字符串中读取字符
int read_char(const char **str, int width, va_list ap, int suppress) {
    int count = width > 0 ? width : 1;
    
    if (!suppress) {
        char *val = va_arg(ap, char*);
        strncpy(val, *str, count);
        val[count] = '\0';
    }
    
    *str += count;
    return count > 0 ? (!suppress ? 1 : 0) : -1;
}

// 主vsscanf函数
int vsscanf(const char *str, const char *format, va_list ap)
{
    int count = 0;
    const char *s = str;
    const char *f = format;
    
    while (*f && *s) {
        if (*f == '%') {
            format_specifier spec;
            if (parse_format_specifier(&f, &spec)) {
                int result = 0;
                
                switch (spec.specifier) {
                    case 'd':
                    case 'i':
                        skip_whitespace(&s);
                        result = read_integer(&s, spec.width, spec.length, 0, ap, spec.suppress);
                        break;
                    case 'o':
                        skip_whitespace(&s);
                        result = read_integer(&s, spec.width, spec.length, 8, ap, spec.suppress);
                        break;
                    case 'x':
                    case 'X':
                        skip_whitespace(&s);
                        result = read_integer(&s, spec.width, spec.length, 16, ap, spec.suppress);
                        break;
                    case 'u':
                        skip_whitespace(&s);
                        result = read_integer(&s, spec.width, spec.length, 10, ap, spec.suppress);
                        break;
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                        skip_whitespace(&s);
                        result = read_float(&s, spec.width, spec.length, ap, spec.suppress);
                        break;
                    case 's':
                        result = read_string(&s, spec.width, ap, spec.suppress);
                        break;
                    case 'c':
                        result = read_char(&s, spec.width, ap, spec.suppress);
                        break;
                    case 'p':
                        skip_whitespace(&s);
                        result = read_pointer(&s, spec.width, ap, spec.suppress);
                        break;
                    case '[':
                        result = read_scanset(&s, spec.width, spec.scanset, 
                                            spec.scanset_invert, ap, spec.suppress);
                        break;
                    case '%':
                        if (*s == '%') {
                            s++;
                            result = 1;
                        }
                        break;
                    default:
                        break;
                }
                
                if (result > 0 && !spec.suppress) {
                    count++;
                } else if (result < 0) {
                    free_format_specifier(&spec);
                    break;  // 解析失败
                }
                
                free_format_specifier(&spec);
            } else {
                f++;  // 跳过无效的%
            }
        } else if (isspace(*f)) {
            // 跳过格式字符串中的空白
            skip_whitespace(&f);
            skip_whitespace(&s);
        } else {
            // 普通字符匹配
            if (*f == *s) {
                f++;
                s++;
            } else {
                break;  // 不匹配
            }
        }
    }
    
    return count;
}

int sscanf(char *s, const char *format, ...)
{
    va_list args;
    int len;
 
    va_start(args, format);
    len = vsscanf(s,format,args);
    va_end(args);
 
    return len;
}

int vscanf(const char *format, va_list ap)
{
    int len;
    char s[1025];
    napi_gets(s,1024);
    len=vsscanf(s,format,ap);
    return len;
}

int scanf(const char *format, ...)
{
    va_list args;
    int len;
 
    va_start(args, format);
    len = vscanf(format,args);
    va_end(args);
 
    return len;
}

int getchar()
{
    return napi_getc();
}
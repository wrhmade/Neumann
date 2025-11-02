#include "malloc.c"//内存分配部分
#include "rand.c"//随机数部分
#include <ctype.h>

// 自定义字符串转long double函数
long double strtold(const char *str, char **endptr)
{
    const char *p = str;
    long double result = 0.0L;
    int sign = 1;
    int exponent_sign = 1;
    long double fraction = 0.0L;
    long double fraction_multiplier = 0.1L;
    int has_digits = 0;
    int has_exponent = 0;
    long exponent = 0;
    
    // 跳过前导空白字符
    while (isspace(*p)) {
        p++;
    }
    
    // 处理符号
    if (*p == '+') {
        sign = 1;
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }
    
    // 解析整数部分
    while (isdigit(*p)) {
        has_digits = 1;
        result = result * 10.0L + (*p - '0');
        p++;
    }
    
    // 解析小数部分
    if (*p == '.') {
        p++;
        while (isdigit(*p)) {
            has_digits = 1;
            fraction += (*p - '0') * fraction_multiplier;
            fraction_multiplier *= 0.1L;
            p++;
        }
        result += fraction;
    }
    
    // 如果没有数字，则不是有效数字
    if (!has_digits) {
        if (endptr) {
            *endptr = (char*)str;
        }
        return 0.0L;
    }
    
    // 解析指数部分
    if (*p == 'e' || *p == 'E') {
        has_exponent = 1;
        p++;
        
        // 解析指数符号
        if (*p == '+') {
            exponent_sign = 1;
            p++;
        } else if (*p == '-') {
            exponent_sign = -1;
            p++;
        }
        
        // 解析指数数字
        while (isdigit(*p)) {
            exponent = exponent * 10 + (*p - '0');
            p++;
        }
    }
    
    // 应用指数
    if (has_exponent) {
        if (exponent_sign == 1) {
            for (long i = 0; i < exponent; i++) {
                result *= 10.0L;
            }
        } else {
            for (long i = 0; i < exponent; i++) {
                result /= 10.0L;
            }
        }
    }
    
    // 应用符号
    result *= sign;
    
    // 设置endptr
    if (endptr) {
        *endptr = (char*)p;
    }
    
    return result;
}

double strtod(const char *str, char **endptr)
{
    return (double)strtold(str, endptr);
}
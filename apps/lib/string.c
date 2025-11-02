/*
string.c 
标准字符串库 
Copyright W24 Studio 
*/
#include <string.h>
#include <stdint.h> 
#include <stddef.h> 
#include <limits.h>
#include <errno.h>
#include <stdlib.h>

void *memset(void *dst_, uint8_t value, uint32_t size)
{
    uint8_t *dst = (uint8_t *) dst_;
    while (size-- > 0) *dst++ = value;
    return dst_;
}
 
void *memcpy(void *dst_, const void *src_, uint32_t size)
{
    uint8_t *dst = dst_;
    const uint8_t *src = src_;
    while (size-- > 0) *dst++ = *src++;
    return (void *) src_;
}
 
int memcmp(const void *a_, const void *b_, uint32_t size)
{
    const char *a = a_;
    const char *b = b_;
    while (size-- > 0) {
        if (*a != *b) return *a > *b ? 1 : -1;
        a++, b++;
    }
    return 0;
}
 
char *strcpy(char *dst_, const char *src_)
{
    char *r = dst_;
    while ((*dst_++ = *src_++));
    return r;
}
 
uint32_t strlen(const char *str)
{
    const char *p = str;
    while (*p++);
    return p - str - 1;
}
 
int8_t strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) a++, b++;
    return *a < *b ? -1 : *a > *b;
}
 
char *strchr(const char *str, const uint8_t ch)
{
    while (*str) {
        if (*str == ch) return (char *) str;
        str++;
    }
    return NULL;
}

void bzero(void *dest, uint32_t len)
{
	memset(dest, 0, len);
}
 
int strncmp(const char *str1, const char *str2, size_t n)
{
    if (n == 0) {
        return 0;
    }
 
    while (--n && *str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
 
    return *str1 - *str2;
}

char* strstr(const char* str1, const char* str2)
{
    const char* s1;
    const char* s2;
    
    if (!*str2) {
        return (char*)str1; // 如果str2为空字符串，则返回str1
    }
 
    while (*str1) {
        s1 = str1;
        s2 = str2;
 
        while (*s1 && *s2 && *s1 == *s2) {
            s1++;
            s2++;
        }
 
        if (!*s2) {
            return (char*)str1; // 找到匹配的子字符串
        }
 
        str1++; // 没有找到，尝试下一个位置
    }
 
    return NULL; // 没有找到匹配的子字符串
}

char *strcat(char *dst_, const char *src_)
{
    char *str = dst_;
    while (*str++);
    --str;
    while((*str++ = *src_++));
    return dst_;
}

char *strncpy(char *dst_, const char *src_, int n)
{
    char *str = dst_;
    while (n && (*dst_++ = *src_++)) n--;
    if (n) while (n--) *dst_++ = '\0';
    return str;
}

char *strtok(char *str, const char *delim)
{
    static char *last;
    char *start;
 
    if (str)
        start = str;
    else {
        start = last;
        if (!start) {
            return NULL;
        }
    }
 
    // 跳过前导分隔符
    while (*start && strchr(delim, *start)) {
        start++;
    }
    if (!*start) {
        return NULL;
    }
 
    // 找到下一个分隔符的位置
    char *end = start + 1;
    while (*end && !strchr(delim, *end)) {
        end++;
    }
 
    // 将分隔符改为'\0'，形成一个新的字符串
    if (*end) {
        *end = '\0';
    }
 
    // 保存本次分解的位置，为下次调用准备
    last = end;
 
    return start;
}

long int strtol(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    long int result = 0;
    unsigned long int limit;
    int negative = 0;

    // 跳过前导空格
    while (*s == ' ' || *s == '\t') {
        s++;
    }

    // 处理符号
    if (*s == '-' || *s == '+') {
        negative = (*s == '-');
        s++;
    }

    // 处理基数
    if (base == 0) {
        if (*s == '0') {
            if (s[1] == 'x' || s[1] == 'X') {
                base = 16;
                s += 2;
            } else {
                base = 8;
                s++;
            }
        } else {
            base = 10;
        }
    } else if (base < 2 || base > 36) {
        return -1;
    }

    // 计算转换结果
    limit = negative ? INT_MIN : INT_MAX;
    while ((*s >= '0' && *s <= '9') ||
           (*s >= 'a' && *s <= 'z') ||
           (*s >= 'A' && *s <= 'Z')) {
        int digit = *s - '0';
        if (*s >= 'a' || *s >= 'A') {
            digit += 10 - (*s < 'a');
        }
        if (digit >= base) {
            break;
        }
        if (result < (limit + digit) / base) {
            return negative ? INT_MIN : INT_MAX;
        }
        result = result * base + digit;
        s++;
    }

    // 设置结束指针
    if (endptr) {
        *endptr = (char *)s;
    }

    return negative ? -result : result;
}

/* 删除字符串中指定位置的字符 */
void delete_char(char *str, int pos)
{
	int i;
	for (i = pos; i < strlen(str); i++) {
		str[i] = str[i + 1];
	}
}

/* 在字符串的指定位置插入一个字符 */
void insert_char(char *str, int pos, char ch)
{
	int i;
	for (i = strlen(str); i >= pos; i--) {
		str[i + 1] = str[i];
	}
	str[pos] = ch;
}

/* 在字符串的指定位置插入另一个字符串 */
void insert_str(char *str, char *insert_str, int pos)
{
	for (int i = 0; i < strlen(insert_str); i++) {
		insert_char(str, pos + i, insert_str[i]);
	}
}

/* 将字符串中的所有字母转换为大写 */
char *strupr(char *src)
{
	while (*src != '\0')
	{
		if (*src >= 'a' && *src <= 'z')
			*src -= 32;
		src++;
	}
	return src;
}

/* 将字符串中的所有字母转换为小写 */
char *strlwr(char *src)
{
	while (*src != '\0')
	{
		if (*src > 'A' && *src <= 'Z'){
			//*src += 0x20; 
			*src += 32;
		}
		src++;
	}
	return src;
}

/* 拷贝字符串副本并返回 */
void *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	void *p = (void *)malloc(len);
	if (p != 0) memcpy(p, (uint8_t *)s, len);
	return p;
}

int streq(char *a,char *b)
{
    return strcmp(a,b)==0;
}

char *strchrnul(const char* s, int c)
{
	char* p = (char*)s;
	while (*p && *p != (char)c) {
		p++;
	}
	return p;
}


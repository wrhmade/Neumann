/*
string.c 
标准字符串库 
Copyright W24 Studio 
*/
#include <string.h>
#include <stdint.h> 
#include <stddef.h> 

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

char* strncpy(char* dest, const char* src, size_t n)
{
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    // 如果源字符串短于n，则目标字符串应以'\0'结尾
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}
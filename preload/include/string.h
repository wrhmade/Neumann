/*
string.h
标准字符串库头文件 
Copyright W24 Studio 
*/

#include <stdint.h> 
#include <stddef.h> 

#ifndef STRING_H
#define STRING_H
 
void *memset(void *dst_, uint8_t value, uint32_t size);
void *memcpy(void *dst_, const void *src_, uint32_t size);
int memcmp(const void *a_, const void *b_, uint32_t size);
char *strcpy(char *dst_, const char *src_);
uint32_t strlen(const char *str);
int8_t strcmp(const char *a, const char *b);
char *strchr(const char *str, const uint8_t ch);
void bzero(void *dest, uint32_t len);
char* strstr(const char* str1, const char* str2);
char* strncpy(char* dest, const char* src, size_t n);
#endif

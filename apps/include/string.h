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
int strncmp(const char *str1, const char *str2, size_t n);
char *strchr(const char *str, const uint8_t ch);
void bzero(void *dest, uint32_t len);
char* strstr(const char* str1, const char* str2);
char *strcat(char *dst_, const char *src_);
char *strncpy(char *dst_, const char *src_, int n);
char *strtok(char *str, const char *delim);
long int strtol(const char *nptr, char **endptr, int base);


void delete_char(char *str, int pos);
void insert_char(char *str, int pos, char ch);
void insert_str(char *str, char *insert_str, int pos);
char *strupr(char *src);
char *strlwr(char *src);
void *strdup(const char *s);

int streq(char *a,char *b);
char *strchrnul(const char* s, int c);
#endif

/*
ini.h
INI配置文件解析头文件
Copyright W24 Studio 
*/

#ifndef INI_H
#define INI_H
char* read_line(const char* str, int line_number);
int countLines(char* str);
int read_ini_buf(char *buf,char* section,char* key,char *value);
int read_ini(char *filename,char* section,char* key,char *value);
#endif
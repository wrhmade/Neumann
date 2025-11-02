/*
ime.c
中文输入法头文件
Copyright W24 Studio 
*/

#ifndef IME_H
#define IME_H
#define IME_MB_MAX 32

typedef struct IME_STATUS
{
    int enabled;//是否开启中文输入
    int inputmode;//输入模式:0:英文输入,1:中文输入
    char *mb;//码表
}ime_status_t;

typedef struct CHINESE//中文字符结构体
{
    char byte1,byte2;
}chinese_t;

typedef struct IME_MB
{
    char name[50];
    int flag;
    int size;
    char *buffer;
}ime_mb_t;

typedef struct IME_CHOOSE_TABLE
{
    int n;
    chinese_t chr[5];
}ime_ct_t;

void ime_init();
int ime_load_mb(const char *filename,const char *mb_name);

#endif
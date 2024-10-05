/*
ime.c
中文输入法头文件
Copyright W24 Studio 
*/

#ifndef IME_H
#define IME_H

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

void ime_init();

#endif
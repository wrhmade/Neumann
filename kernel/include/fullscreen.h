/*
fullscreen.h
全屏界面头文件
Copyright W24 Studio 
*/

#ifndef FULLSCREEN_H
#define FULLSCREEN_H
#include <stdint.h>
void fullscreen_init();
void fullscreen_show();
int get_isfullscreen();
uint32_t *get_fullscreen_buffer();
void fullscreen_refresh();
void fullscreen_hide();
#endif
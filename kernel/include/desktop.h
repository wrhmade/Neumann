/*
desktop.h 
桌面环境头文件
Copyright W24 Studio 
*/

#ifndef DESKTOP_H
#define DESKTOP_H
#include <stdint.h>
void desktop_reload(char *themename);
void init_desktop(uint32_t *buf,uint32_t xsize,uint32_t ysize);
void draw_mouse(uint32_t *buf_mouse);
int load_wallpaper(uint32_t *vram,int x,int y,char *wallpaper_name);
#endif
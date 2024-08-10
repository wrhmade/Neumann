/*
window.h 
窗口管理头文件
Copyright W24 Studio 
*/

#ifndef WINDOW_H
#define WINDOW_H
#include <stdint.h>
typedef struct SHEET sheet_t;;
typedef struct WINDOW
{
    sheet_t *sheet;
    uint32_t xsize,ysize;
    char title[60];
}window_t;

window_t *create_window(char *title,uint32_t xsize,uint32_t ysize,uint32_t col_inv);
void window_init(window_t *window,sheet_t *sheet,uint32_t xsize,uint32_t ysize,char *title);
void draw_window(window_t *window);
void show_window(window_t *window);
void hide_window(window_t *window);
void move_window(window_t *window,int x,int y);
void close_window(window_t *window);
#endif
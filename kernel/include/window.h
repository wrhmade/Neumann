/*
window.h 
窗口管理头文件
Copyright W24 Studio 
*/

#ifndef WINDOW_H
#define WINDOW_H
#include <stdint.h>
#include <task.h>
typedef struct SHEET sheet_t;
typedef struct CONSOLE console_t;
typedef struct WINDOW
{
    sheet_t *sheet;
    uint32_t xsize,ysize;
    char title[60];
    task_t *task;
    int isconsole;
    console_t *console;
    uint32_t close_btn;
}window_t;
void window_redraw();
window_t *create_window(char *title,uint32_t xsize,uint32_t ysize,uint32_t col_inv,uint32_t close_btn);
void window_init(window_t *window,sheet_t *sheet,uint32_t xsize,uint32_t ysize,char *title);
void draw_window(window_t *window,int title_only);
void show_window(window_t *window);
void hide_window(window_t *window);
void move_window(window_t *window,int x,int y);
void close_window(window_t *window);
void window_settask(window_t *window,task_t* task);
#endif
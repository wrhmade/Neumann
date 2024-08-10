/*
window.c 
窗口管理
Copyright W24 Studio 
*/

#include <window.h>
#include <sheet.h>
#include <mm.h>
#include <string.h>
#include <graphic.h>

#define WINDOW_COLOR 0xFAFAFA
#define WINDOW_TITLE_COLOR 0x07689F
#define WINDOW_CLOSE_BUTTON_COLOR 0xFAFAFA
#define WINDOW_CLOSE_BUTTON_BACKCOLOR 0xFF0000
extern shtctl_t *global_shtctl;

window_t *create_window(char *title,uint32_t xsize,uint32_t ysize,uint32_t col_inv)
{
    window_t *window=(window_t *)malloc(sizeof(window_t));
    sheet_t *sheet_window;
	uint32_t *buf_window;
	sheet_window=sheet_alloc(global_shtctl);
	buf_window=(uint32_t *)malloc(sizeof(uint32_t)*xsize*ysize);
	sheet_setbuf(sheet_window,buf_window,xsize,ysize,-1);
    window_init(window,sheet_window,xsize,ysize,title);
    draw_window(window);
	sheet_updown(window->sheet,1);
	sheet_slide(window->sheet,100,100);
    return window;
}

void window_init(window_t *window,sheet_t *sheet,uint32_t xsize,uint32_t ysize,char *title)
{
    window->sheet=sheet;
    sheet->window=window;
    window->xsize=xsize;
    window->ysize=ysize;
    strcpy(window->title,title);
}

void draw_window(window_t *window)
{
    boxfill(window->sheet->buf,window->xsize,0,0,window->xsize-1,17,WINDOW_TITLE_COLOR);
    boxfill(window->sheet->buf,window->xsize,0,18,window->xsize-1,window->ysize-1,WINDOW_COLOR);
    boxfill(window->sheet->buf,window->xsize,0,0,0,window->ysize-1,0x000000);
    boxfill(window->sheet->buf,window->xsize,0,0,window->xsize-1,0,0x000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize-1,0,window->xsize-1,window->ysize-1,0x000000);
    boxfill(window->sheet->buf,window->xsize,0,window->ysize-1,window->xsize-1,window->ysize-1,0x000000);
    boxfill(window->sheet->buf,window->xsize,0,18,window->xsize-1,18,0x000000);
    putstr_ascii(window->sheet->buf,window->xsize,1,1,0xFFFFFF,window->title);

    static char btn_close_chr[16][16]={
        "................",
        "................",
        "................",
        "..***......***..",
        "...***....***...",
        "....***..***....",
        ".....******.....",
        "......****......",
        "......****......",
        ".....******.....",
        "....***..***....",
        "...***....***...",
        "..***......***..",
        "................",
        "................",
        "................"
    };
    uint32_t *btn_close=(uint32_t *)malloc(sizeof(uint32_t)*16*16);
    int i,j;
    for(i=0;i<16;i++)
    {
        for(j=0;j<16;j++)
        {
            if(btn_close_chr[i][j]=='*')
            {
                btn_close[i*16+j]=WINDOW_CLOSE_BUTTON_COLOR;
            }
            if(btn_close_chr[i][j]=='.')
            {
                btn_close[i*16+j]=WINDOW_CLOSE_BUTTON_BACKCOLOR;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,16,16,window->xsize-17,1,btn_close,16);
    sheet_refresh(window->sheet,0,0,window->xsize,window->ysize);
}

void show_window(window_t *window)
{
    sheet_updown(window->sheet,1);
    sheet_refresh(window->sheet,0,0,window->xsize,window->ysize);
}

void hide_window(window_t *window)
{
    sheet_updown(window->sheet,-1);
    sheet_refresh(window->sheet,0,0,window->xsize,window->ysize);
}

void move_window(window_t *window,int x,int y)
{
    sheet_slide(window->sheet,x,y);
    sheet_refresh(window->sheet,0,0,window->xsize,window->ysize);
}

void close_window(window_t *window)
{
    hide_window(window);
    free(window->sheet->buf);
    sheet_free(window->sheet);
    free(window);
}
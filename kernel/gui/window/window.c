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
    window->xsize=xsize;
    window->ysize=ysize;
    strcpy(window->title,title);
}

void draw_window(window_t *window)
{
    boxfill(window->sheet->buf,window->xsize,0,0,window->xsize,window->ysize,0xE6E6E6);
    boxfill(window->sheet->buf,window->xsize,0,0,window->xsize,18,0x808080);
    putstr_ascii_sheet(window->sheet,1,1,0xFFFFFF,0x808080,window->title);
	

    static char btn_close_chr[16][16]={
        "................",
        "................",
        "................",
        "...*........*...",
        "....*......*....",
        ".....*....*.....",
        "......*..*......",
        ".......**.......",
        ".......**.......",
        "......*..*......",
        ".....*....*.....",
        "....*......*....",
        "...*........*...",
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
            if(btn_close_chr[i][j]=='.')
            {
                btn_close[i*16+j]=0xFFFFFF;
            }
            if(btn_close_chr[i][j]=='.')
            {
                btn_close[i*16+j]=0xFF0000;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,16,16,window->xsize-17,2,btn_close,16);
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
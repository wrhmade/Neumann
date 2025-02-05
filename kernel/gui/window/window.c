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
#include <task.h>

uint32_t WINDOW_COLOR=0xFAFAFA;
uint32_t WINDOW_TITLE_COLOR=0x07689F;
uint32_t WINDOW_TITLE_FCOLOR=0xFFFFFF;
uint32_t WINDOW_CLOSE_BUTTON_COLOR=0xFAFAFA;
uint32_t WINDOW_CLOSE_BUTTON_BACKCOLOR=0xFF0000;
extern shtctl_t *global_shtctl;


void window_redraw()
//重画所有窗口
{
    int i;
    sheet_t *sht;
    window_t *win;
    for(i=global_shtctl->top-1;i>0;i--)
    {
        sht=global_shtctl->sheets[i];
        if(sht->window!=NULL)//这个图层是窗口
        {
            draw_window(sht->window,1);
        }
    }
}

window_t *create_window(char *title,uint32_t xsize,uint32_t ysize,uint32_t col_inv,uint32_t close_btn)
{
    window_t *window=(window_t *)malloc(sizeof(window_t));
    sheet_t *sheet_window;
	uint32_t *buf_window;
	sheet_window=sheet_alloc(global_shtctl);
	buf_window=(uint32_t *)malloc(sizeof(uint32_t)*xsize*ysize);
	sheet_setbuf(sheet_window,buf_window,xsize,ysize,-1);
    window_init(window,sheet_window,xsize,ysize,title);
    window->close_btn=close_btn;
    draw_window(window,0);
	sheet_updown(window->sheet,1);
	sheet_slide(window->sheet,100,100);
    return window;
}

void window_init(window_t *window,sheet_t *sheet,uint32_t xsize,uint32_t ysize,char *title)
{
    window->sheet=sheet;
    sheet->window=window;
    sheet->window=window;
    window->xsize=xsize;
    window->ysize=ysize;
    window->task=NULL;
    window->console=NULL;
    window->isconsole=0;
    strcpy(window->title,title);
}

void draw_window(window_t *window,int title_only)
{
    boxfill(window->sheet->buf,window->xsize,0,0,window->xsize-1,17,WINDOW_TITLE_COLOR);
    if(!title_only)boxfill(window->sheet->buf,window->xsize,0,18,window->xsize-1,window->ysize-1,WINDOW_COLOR);
    boxfill(window->sheet->buf,window->xsize,0,0,0,window->ysize-1,0x000000);
    boxfill(window->sheet->buf,window->xsize,0,0,window->xsize-1,0,0x000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize-1,0,window->xsize-1,window->ysize-1,0x000000);
    boxfill(window->sheet->buf,window->xsize,0,window->ysize-1,window->xsize-1,window->ysize-1,0x000000);
    boxfill(window->sheet->buf,window->xsize,0,18,window->xsize-1,18,0x000000);
    putstr_ascii(window->sheet->buf,window->xsize,1,1,WINDOW_TITLE_FCOLOR,window->title);

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
                btn_close[i*16+j]=WINDOW_CLOSE_BUTTON_COLOR;
            }
            if(btn_close_chr[i][j]=='.')
            {
                btn_close[i*16+j]=WINDOW_CLOSE_BUTTON_BACKCOLOR;
                btn_close[i*16+j]=WINDOW_CLOSE_BUTTON_BACKCOLOR;
            }
        }
    }
    if(window->close_btn)putblock(window->sheet->buf,window->xsize,16,16,window->xsize-17,2,btn_close,16);
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

void window_settask(window_t *window,task_t* task)
{
    window->task=task;
    task->window=window;
}
/*
msdemo.c
鼠标辅助演示程序
Copyright W24 Studio 
*/

#include <msdemo.h>
#include <window.h>
#include <sheet.h>
#include <graphic.h>
#include <stdio.h>

extern int global_mousebtn,mouse_x,mouse_y;
int mousebtn,mx,my,dx,dy,moving;

static void refresh(void)
{
    task_t *task=task_now();
    window_t *window=task->window;
    boxfill(window->sheet->buf,window->xsize,49,67,window->xsize-49,window->ysize-49,0xFF000000);
    boxfill(window->sheet->buf,window->xsize,50,68,window->xsize-50,window->ysize-50,0xFFFFFFFF);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-50/2-1,67,window->xsize/2+50/2+1,161,0xFF000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-50/2,68,window->xsize/2+50/2,160,0xFFFFFFFF);
    boxfill(window->sheet->buf,window->xsize,50,200,window->xsize-50,200,0xFF000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2,161,window->xsize/2,200,0xFF000000);
    char s[50];
    boxfill(window->sheet->buf,window->xsize,1,19,18*8+1,18+16,0xFFFFFFFF);
    sprintf(s,"鼠标位置:(%d,%d)",mouse_x,mouse_y);
    putstr_ascii(window->sheet->buf,window->xsize,1,19,0xFF000000,s);

    if((mousebtn&0x04)!=0)
    {
        boxfill(window->sheet->buf,window->xsize,window->xsize/2-50/2,68,window->xsize/2+50/2,160,0xFF808080);
    }
    if((mousebtn&0x01)!=0)
    {
        boxfill(window->sheet->buf,window->xsize,50,68,window->xsize/2-50/2-2,161,0xFF808080);
        boxfill(window->sheet->buf,window->xsize,50,162,window->xsize/2-1,199,0xFF808080);
    }
    if((mousebtn&0x02)!=0)
    {
        boxfill(window->sheet->buf,window->xsize,window->xsize/2+50/2+2,68,window->xsize-50,161,0xFF808080);
        boxfill(window->sheet->buf,window->xsize,window->xsize/2+1,162,window->xsize-50,199,0xFF808080);
    }

    if(dx==0)//静止
    {
        putstr_ascii(window->sheet->buf,window->xsize,25-8,(window->ysize-18)/2-8,0xFF000000,"左");
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize-50/2-8,(window->ysize-18)/2-8,0xFF000000,"右");
    }
    else if(dx>0)//向右移动
    {
        putstr_ascii(window->sheet->buf,window->xsize,25-8,(window->ysize-18)/2-8,0xFF000000,"左");
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize-50/2-8,(window->ysize-18)/2-8,0xFFFF0000,"右");
    }
    else if(dx<0)//向左移动
    {
        putstr_ascii(window->sheet->buf,window->xsize,25-8,(window->ysize-18)/2-8,0xFFFF0000,"左");
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize-50/2-8,(window->ysize-18)/2-8,0xFF000000,"右");
    }
    if(dy==0)//静止
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-8,25+18,0xFF000000,"上");
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-8,window->ysize-25,0xFF000000,"下");
    }
    else if(dy>0)//向下运动
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-8,25+18,0xFF000000,"上");
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-8,window->ysize-25,0xFFFF0000,"下");
    }
    else if(dy<0)//向上运动
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-8,25+18,0xFFFF0000,"上");
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-8,window->ysize-25,0xFF000000,"下");
    }

    sheet_refresh(window->sheet,0,0,window->xsize-1,window->ysize-1);
}

void msdemo_main()
{
    refresh();
    mx=mouse_x;
    my=mouse_y;
    moving=0;
    for(;;)
    {
        if(mousebtn!=global_mousebtn)
        {
            mousebtn=global_mousebtn;
            refresh();
        }
        if(mx!=mouse_x)
        {
            dx=mouse_x-mx;
            mx=mouse_x;
            moving=1;
            refresh();
        }
        if(my!=mouse_y)
        {
            dy=mouse_y-my;
            my=mouse_y;
            moving=1;
            refresh();
        }
        if(my==mouse_y || mx==mouse_x)
        {
            if(moving)
            {
                dx=dy=0;
                moving=0;
                refresh();
            }
        }
    }
}

void start_msdemo()
{
    window_t *window=create_window("鼠标演示辅助程序",350,400,-1,1);
    task_t *task=create_kernel_task(msdemo_main);
    window_settask(window,task);
    task->langmode=1;
    task_run(task);
}
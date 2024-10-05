/*
message.c
弹窗消息
Copyright W24 Studio 
*/

#include <message.h>
#include <window.h>
#include <task.h>
#include <buzzer.h>
#include <timer.h>
#include <binfo.h>
#include <macro.h>
#include <graphic.h>
#include <sheet.h>

extern int global_mousebtn,mouse_x,mouse_y;






void message_main()
//消息处理主任务程序
{
    task_t *task=task_now();
    window_t *window=task->window;
    int i;
    beep(900000);
    sleep(10);
    beep(0);
    for(;;)
    {
        if(mouse_x>=window->sheet->vx0+(window->xsize/2-32/2-2) && mouse_x<=window->sheet->vx0+(window->xsize/2+32/2+2))
        {
            if(mouse_y>=window->sheet->vy0+80 && mouse_y<=window->sheet->vy0+82+16+2 && (global_mousebtn & 0x01)!=0)
            {//点击确定按钮
                
                close_window(window);
                task_exit(0);
            }
        }
        if(fifo_status(&task->fifo)>0)
        {
            i=fifo_get(&task->fifo);
            if(i-256==0x20)//按下空格键
            {
                close_window(window);
                task_exit(0);
            }
            if(i-256==0x0A)//按下回车键
            {
                close_window(window);
                task_exit(0);
            }
        }
    }  
}

void error_message(char *content,char *title)
{
    static char error_icon[32][32]=
    {
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRWWWWWWWWWWWWWWWWWWRRRRRBBB",
	    "BBBRRRWWWWWWWWWWWWWWWWWWRRRRRBBB",
	    "BBBRRRWWWWWWWWWWWWWWWWWWGGRRRBBB",
	    "BBBRRRWWWWWWWWWWWWWWWWWWGGRRRBBB",
	    "BBBRRRRRRGGGGGGGGGGGGGGGGGRRRBBB",
	    "BBBRRRRRRGGGGGGGGGGGGGGGGGRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB", 
	    "BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
    	"BBBRRRRRRRRRRRRRRRRRRRRRRRRRRBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    window_t *window=create_window(title,strlen(content)*8+20+64,64+18+32,-1,0);
    task_t *task_n=task_now();
    hide_window(window);
    putstr_ascii(window->sheet->buf,window->xsize,64,44,0x000000,content);

    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2-2,80,window->xsize/2+32/2+2,82+16+2,0x0000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,window->xsize/2+32/2,82+16,0xFFFFFF);
    if(task_n->langmode==1 || task_n->langmode==2)
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0x000000,"确定");
    }
    else
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0x000000," OK ");
    }

    uint32_t *error_icon_grap=(uint32_t)malloc(sizeof(uint32_t)*32*32);
    int x,y;

    for(y=0;y<32;y++)
    {
        for(x=0;x<32;x++)
        {
            if(error_icon[y][x]=='B')
            {
                error_icon_grap[y*32+x]=0x000000;
            }
            if(error_icon[y][x]=='R')
            {
                error_icon_grap[y*32+x]=0xFF0000;
            }
            if(error_icon[y][x]=='W')
            {
                error_icon_grap[y*32+x]=0xFFFFFF;
            }
            if(error_icon[y][x]=='G')
            {
                error_icon_grap[y*32+x]=0x808080;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,32,32,16,36,error_icon_grap,32);
    
    task_t *task=create_kernel_task(message_main);
    window_settask(window,task);
    move_window(window,binfo->scrnx/2-window->xsize/2,binfo->scrny/2-window->ysize/2);
    show_window(window);
    task_run(task);
    free(error_icon_grap);
}

void warn_message(char *content,char *title)
{
    static char warn_icon[32][32]=
    {
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYBBYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYBBBBYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYBBBBBBYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYBBBBYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYBBBBYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYBBBBYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYBBBBYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYBBYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYBBYYYYYYYYYYYYBBB",
    	"BBBYYYYYYYYYYYBBBBYYYYYYYYYYYBBB",
    	"BBBYYYYYYYYYYYBBBBYYYYYYYYYYYBBB",
        "BBBYYYYYYYYYYYYBBYYYYYYYYYYYYBBB",
    	"BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    window_t *window=create_window(title,strlen(content)*8+20+64,64+18+32,-1,0);
    task_t *task_n=task_now();
    hide_window(window);
    putstr_ascii(window->sheet->buf,window->xsize,64,44,0x000000,content);

    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2-2,80,window->xsize/2+32/2+2,82+16+2,0x0000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,window->xsize/2+32/2,82+16,0xFFFFFF);
    if(task_n->langmode==1 || task_n->langmode==2)
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0x000000,"确定");
    }
    else
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0x000000," OK ");
    }

    uint32_t *warn_icon_grap=(uint32_t)malloc(sizeof(uint32_t)*32*32);
    int x,y;

    for(y=0;y<32;y++)
    {
        for(x=0;x<32;x++)
        {
            if(warn_icon[y][x]=='B')
            {
                warn_icon_grap[y*32+x]=0x000000;
            }
            if(warn_icon[y][x]=='Y')
            {
                warn_icon_grap[y*32+x]=0xFFFF00;
            }
            if(warn_icon[y][x]=='W')
            {
                warn_icon_grap[y*32+x]=0xFFFFFF;
            }
            if(warn_icon[y][x]=='G')
            {
                warn_icon_grap[y*32+x]=0x808080;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,32,32,16,36,warn_icon_grap,32);
    
    task_t *task=create_kernel_task(message_main);
    window_settask(window,task);
    move_window(window,binfo->scrnx/2-window->xsize/2,binfo->scrny/2-window->ysize/2);
    show_window(window);
    task_run(task);
    free(warn_icon_grap);
}

void info_message(char *content,char *title)
{
    static char info_icon[32][32]=
    {
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYWWYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYWWYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYWWYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYWWWWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
    	"BBBYYYYYYYYYYYWWWWYYYYYYYYYYYBBB",
    	"BBBYYYYYYYYWWWWWWWWWWWYYYYYYYBBB",
        "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
    	"BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBYYYYYYYYYYYYYYYYYYYYYYYYYYBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
	    "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
    };
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    window_t *window=create_window(title,strlen(content)*8+20+64,64+18+32,-1,0);
    task_t *task_n=task_now();
    hide_window(window);
    putstr_ascii(window->sheet->buf,window->xsize,64,44,0x000000,content);

    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2-2,80,window->xsize/2+32/2+2,82+16+2,0x0000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,window->xsize/2+32/2,82+16,0xFFFFFF);
    if(task_n->langmode==1 || task_n->langmode==2)
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0x000000,"确定");
    }
    else
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0x000000," OK ");
    }

    uint32_t *info_icon_grap=(uint32_t)malloc(sizeof(uint32_t)*32*32);
    int x,y;

    for(y=0;y<32;y++)
    {
        for(x=0;x<32;x++)
        {
            if(info_icon[y][x]=='B')
            {
                info_icon_grap[y*32+x]=0x000000;
            }
            if(info_icon[y][x]=='Y')
            {
                info_icon_grap[y*32+x]=0x0000FF;
            }
            if(info_icon[y][x]=='W')
            {
                info_icon_grap[y*32+x]=0xFFFFFF;
            }
            if(info_icon[y][x]=='G')
            {
                info_icon_grap[y*32+x]=0x808080;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,32,32,16,36,info_icon_grap,32);
    
    task_t *task=create_kernel_task(message_main);
    window_settask(window,task);
    move_window(window,binfo->scrnx/2-window->xsize/2,binfo->scrny/2-window->ysize/2);
    show_window(window);
    task_run(task);
    free(info_icon_grap);
}
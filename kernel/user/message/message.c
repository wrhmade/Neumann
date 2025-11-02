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
extern shtctl_t *global_shtctl;
extern sheet_t *sht_mouse;


void message_main()
//消息处理主任务程序
{
    task_t *task=task_now();
    window_t *window=task->window;
    sheet_updown(window->sheet,global_shtctl->top-1);
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
    putstr_ascii(window->sheet->buf,window->xsize,64,44,0xFF000000,content);

    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2-2,80,window->xsize/2+32/2+2,82+16+2,0xFF000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,window->xsize/2+32/2,82+16,0xFFFFFFFF);
    if(task_n->langmode==1 || task_n->langmode==2)
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0xFF000000,"确定");
    }
    else
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0xFF000000," OK ");
    }

    uint32_t *error_icon_grap=(uint32_t *)kmalloc(sizeof(uint32_t)*32*32);
    int x,y;

    for(y=0;y<32;y++)
    {
        for(x=0;x<32;x++)
        {
            if(error_icon[y][x]=='B')
            {
                error_icon_grap[y*32+x]=0xFF000000;
            }
            if(error_icon[y][x]=='R')
            {
                error_icon_grap[y*32+x]=0xFFFF0000;
            }
            if(error_icon[y][x]=='W')
            {
                error_icon_grap[y*32+x]=0xFFFFFFFF;
            }
            if(error_icon[y][x]=='G')
            {
                error_icon_grap[y*32+x]=0xFF808080;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,32,32,16,36,error_icon_grap,32);
    
    task_t *task=create_kernel_task(message_main);
    window_settask(window,task);
    move_window(window,binfo->scrnx/2-window->xsize/2,binfo->scrny/2-window->ysize/2);
    show_window(window);
    sheet_updown(window->sheet,sht_mouse->height);
    task_run(task);
    kfree(error_icon_grap);
    task_wait(task_pid(task));
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
    putstr_ascii(window->sheet->buf,window->xsize,64,44,0xFF000000,content);

    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2-2,80,window->xsize/2+32/2+2,82+16+2,0xFF000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,window->xsize/2+32/2,82+16,0xFFFFFFFF);
    if(task_n->langmode==1 || task_n->langmode==2)
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0xFF000000,"确定");
    }
    else
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0xFF000000," OK ");
    }

    uint32_t *warn_icon_grap=(uint32_t *)kmalloc(sizeof(uint32_t)*32*32);
    int x,y;

    for(y=0;y<32;y++)
    {
        for(x=0;x<32;x++)
        {
            if(warn_icon[y][x]=='B')
            {
                warn_icon_grap[y*32+x]=0xFF000000;
            }
            if(warn_icon[y][x]=='Y')
            {
                warn_icon_grap[y*32+x]=0xFFFFFF00;
            }
            if(warn_icon[y][x]=='W')
            {
                warn_icon_grap[y*32+x]=0xFFFFFFFF;
            }
            if(warn_icon[y][x]=='G')
            {
                warn_icon_grap[y*32+x]=0xFF808080;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,32,32,16,36,warn_icon_grap,32);
    
    task_t *task=create_kernel_task(message_main);
    window_settask(window,task);
    move_window(window,binfo->scrnx/2-window->xsize/2,binfo->scrny/2-window->ysize/2);
    show_window(window);
    sheet_updown(window->sheet,sht_mouse->height);
    task_run(task);
    kfree(warn_icon_grap);
    task_wait(task_pid(task));
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
    putstr_ascii(window->sheet->buf,window->xsize,64,44,0xFF000000,content);

    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2-2,80,window->xsize/2+32/2+2,82+16+2,0xFF000000);
    boxfill(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,window->xsize/2+32/2,82+16,0xFFFFFFFF);
    if(task_n->langmode==1 || task_n->langmode==2)
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0xFF000000,"确定");
    }
    else
    {
        putstr_ascii(window->sheet->buf,window->xsize,window->xsize/2-32/2,82,0xFF000000," OK ");
    }

    uint32_t *info_icon_grap=(uint32_t *)kmalloc(sizeof(uint32_t)*32*32);
    int x,y;

    for(y=0;y<32;y++)
    {
        for(x=0;x<32;x++)
        {
            if(info_icon[y][x]=='B')
            {
                info_icon_grap[y*32+x]=0xFF000000;
            }
            if(info_icon[y][x]=='Y')
            {
                info_icon_grap[y*32+x]=0xFF0000FF;
            }
            if(info_icon[y][x]=='W')
            {
                info_icon_grap[y*32+x]=0xFFFFFFFF;
            }
            if(info_icon[y][x]=='G')
            {
                info_icon_grap[y*32+x]=0xFF808080;
            }
        }
    }
    putblock(window->sheet->buf,window->xsize,32,32,16,36,info_icon_grap,32);
    
    task_t *task=create_kernel_task(message_main);
    window_settask(window,task);
    move_window(window,binfo->scrnx/2-window->xsize/2,binfo->scrny/2-window->ysize/2);
    show_window(window);
    sheet_updown(window->sheet,sht_mouse->height);
    task_run(task);
    kfree(info_icon_grap);
    task_wait(task_pid(task));
}

#define MESSAGE_TIP_XSIZE 320
#define MESSAGE_TIP_YSIZE 320

mtman_t mtman[MTMAN_MAX];

static void message_tip_draw(const char *title,const char *content,int index)
{
    boxfill(mtman[index].message_tip_buf,MESSAGE_TIP_XSIZE,0,0,MESSAGE_TIP_XSIZE-1,MESSAGE_TIP_YSIZE-1,0xFFAAAAAA);
    boxfill(mtman[index].message_tip_buf,MESSAGE_TIP_XSIZE,0,0,MESSAGE_TIP_XSIZE-1,15,0xFF888888);
    putstr_ascii(mtman[index].message_tip_buf,MESSAGE_TIP_XSIZE,0,0,0xFF000000,(char *)title);
    int len=strlen(content);
    int x=0,y=16;
    char s[2];
    for(int i=0;i<len;i++)
    {
        s[0]=content[i];
        s[1]=0;
        putstr_ascii(mtman[index].message_tip_buf,MESSAGE_TIP_XSIZE,x,y,0xFF000000,s);
        x+=8;
        if(x>=MESSAGE_TIP_XSIZE)
        {
            x=0;
            y+=16;
        }
    }
}

void message_tip_taskmain(int index)
{
    sheet_slide(mtman[index].message_tip_sht,0,-MESSAGE_TIP_YSIZE);
    sheet_updown(mtman[index].message_tip_sht,global_shtctl->top-1);

    for(int i=0;i<MESSAGE_TIP_YSIZE/10;i++)
    {
        sheet_slide(mtman[index].message_tip_sht,0,mtman[index].message_tip_sht->vy0+10*(index+1));
        sleep(10000000);
    }
    sleep(5000000000);
    for(int i=0;i<MESSAGE_TIP_YSIZE/10;i++)
    {
        sheet_slide(mtman[index].message_tip_sht,0,mtman[index].message_tip_sht->vy0-10*(index+1));
        sleep(10000000);
    }
    task_exit(0);
}

void message_tip_show_task(const char *title,const char *content)
{

    int mtman_index=-1;
    for(int i=0;i<MTMAN_MAX;i++)
    {
        if(mtman[i].flag==0)
        {
            mtman_index=i;
            mtman[i].flag=1;
            break;
        }
    }

    if(mtman_index==-1)
    {
        return;
    }

    mtman[mtman_index].message_tip_buf=(uint32_t *)kmalloc(sizeof(uint32_t)*MESSAGE_TIP_XSIZE*MESSAGE_TIP_YSIZE);
    message_tip_draw(title,content,mtman_index);
    mtman[mtman_index].message_tip_sht=sheet_alloc(global_shtctl);
    sheet_setbuf(mtman[mtman_index].message_tip_sht,mtman[mtman_index].message_tip_buf,MESSAGE_TIP_XSIZE,MESSAGE_TIP_YSIZE,-1);
    mtman[mtman_index].message_tip_sht->movable=0;
    sheet_updown(mtman[mtman_index].message_tip_sht,-1);
    mtman[mtman_index].task=create_kernel_task(message_tip_taskmain);
    name_task(mtman[mtman_index].task,"消息提示");

    mtman[mtman_index].task->tss.esp-=4;
    *((int *)(mtman[mtman_index].task->tss.esp+4))=mtman_index;

    task_run(mtman[mtman_index].task);
    task_wait(task_pid(mtman[mtman_index].task));
    
    sheet_updown(mtman[mtman_index].message_tip_sht,-1);
    sheet_free(mtman[mtman_index].message_tip_sht);
    kfree(mtman[mtman_index].message_tip_buf);

    mtman[mtman_index].flag=0;
    task_exit(0);
}

void message_tip_show(const char *title,const char *content)
{
    task_t *task=create_kernel_task(message_tip_show_task);
    name_task(task,"消息提示附加任务");
    task->tss.esp-=8;
    task->langmode=task_now()->langmode;
    *((uint32_t *)(task->tss.esp+4))=(uint32_t )title;
    *((uint32_t *)(task->tss.esp+8))=(uint32_t )content;
    task_run(task);
}
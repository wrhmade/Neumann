/*
fullscreen.c
全屏界面
Copyright W24 Studio 
*/

#include <sheet.h>
#include <macro.h>
#include <binfo.h>
#include <graphic.h>
#include <mm.h>
#include <fullscreen.h>

static int inited=0,isfullscreen;
extern shtctl_t *global_shtctl;
sheet_t *fullscreen_sht;
uint32_t *fullscreen_buf;

#define TURE_RETURN(n) if(n)return;
#define TURE_RETURN_VALUE(n,v) if(n)return v;
#define FALSE_RETURN(n) if(!n)return;
#define FALSE_RETURN_VALUE(n,v) if(!n)return v;

static void fullscreen_graphic_init()
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    boxfill(fullscreen_buf,binfo->scrnx,0,0,binfo->scrnx-1,binfo->scrny-1,0xFF000000);
}

//初始化全屏界面
void fullscreen_init()
{
    fullscreen_sht=sheet_alloc(global_shtctl);
    fullscreen_sht->movable=0;
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    fullscreen_buf=(uint32_t *)kmalloc(sizeof(uint32_t)*binfo->scrnx*binfo->scrny);
    sheet_setbuf(fullscreen_sht,fullscreen_buf,binfo->scrnx,binfo->scrny,-1);
    fullscreen_graphic_init();
    sheet_updown(fullscreen_sht,-1);
    sheet_slide(fullscreen_sht,0,0);
    inited=1;
    isfullscreen=0;
}

//获取是否全屏
int get_isfullscreen()
{
    return isfullscreen;
}

//显示全屏
void fullscreen_show()
{
    TURE_RETURN(isfullscreen);
    if(!inited)
    {
        fullscreen_init();
    }
    fullscreen_graphic_init();
    sheet_updown(fullscreen_sht,global_shtctl->top);
    isfullscreen=1;
}

//获取全屏显存
uint32_t *get_fullscreen_buffer()
{
    return isfullscreen?fullscreen_buf:NULL;
}

//刷新全屏界面
void fullscreen_refresh()
{
    sheet_refresh(fullscreen_sht,0,0,fullscreen_sht->bxsize-1,fullscreen_sht->bysize-1);
}

//关闭全屏
void fullscreen_hide()
{
    sheet_updown(fullscreen_sht,-1);
    isfullscreen=0;
}
/*
dbuffer.c
双缓冲
Copyright W24 Studio 
*/

#include <dbuffer.h>
#include <binfo.h>
#include <macro.h>
#include <graphic.h>
#include <mm.h>
#include <stdint.h>
#include <string.h>

#pragma GCC optimize("00") //硬件处理不开优化

#define NO_DOUBLEBUFFER 1//禁用双缓冲

#define BINFO_DEF struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO
static int dbuffer_inited=0;

//初始化双缓冲
void init_dbuffer()
{
    if(NO_DOUBLEBUFFER)return;
    BINFO_DEF;
    binfo->vram_really=binfo->vram;
    binfo->vram=(uint32_t *)kmalloc(sizeof(uint32_t)*binfo->scrnx*binfo->scrny);
    memcpy(binfo->vram,binfo->vram_really,sizeof(uint32_t)*binfo->scrnx*binfo->scrny);
    dbuffer_inited=1;
}

void dbuffer_refresh()
{
    if(!dbuffer_inited)return;
    BINFO_DEF;
    uint8_t *dst=(uint8_t *)binfo->vram_really;
    uint8_t *src=(uint8_t *)binfo->vram;
    uint32_t count=(binfo->scrnx*binfo->scrny*sizeof(uint32_t))/4;

    asm volatile("rep movsl"
    : "+D"(dst),"+S"(src),"+c"(count)
    :: "memory"
    );
}
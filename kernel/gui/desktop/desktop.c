/*
desktop.c 
桌面环境
Copyright W24 Studio 
*/

#include <desktop.h>
#include <sysset.h>
#include <graphic.h>
#include <macro.h>
#include <binfo.h>

void init_desktop()
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    boxfill(binfo->vram,binfo->scrnx,0,0,binfo->scrnx,binfo->scrny,DESKTOP_BACKCOLOR);

    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx-24*8,binfo->scrny-77,ARGB(255,0,0,0),"Neumann Operating System");
    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx-19*8,binfo->scrny-61,ARGB(255,0,0,0),"Version 0.8[Beta 6]");
    putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx-52*8,binfo->scrny-45,ARGB(255,0,0,0),"WARNING:THIS IS A BETA VERSION THAT MAY BE UNSTABLE.");
}
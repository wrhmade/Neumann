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

void init_desktop(uint32_t *buf,uint32_t xsize,uint32_t ysize)
{
    boxfill(buf,xsize,0,0,xsize,ysize,DESKTOP_BACKCOLOR);

    putstr_ascii(buf,xsize,xsize-24*8,ysize-77,ARGB(255,0,0,0),"Neumann Operating System");
    putstr_ascii(buf,xsize,xsize-19*8,ysize-61,ARGB(255,0,0,0),"Version 0.8[Beta 6]");
    putstr_ascii(buf,xsize,xsize-52*8,ysize-45,ARGB(255,0,0,0),"WARNING:THIS IS A BETA VERSION THAT MAY BE UNSTABLE.");
}
/*
theme.c
主题
Copyright W24 Studio 
*/

#include <theme.h>
#include <stdint.h>
#include <ini.h>
#include <stddef.h>
#include <string.h>
#include <binfo.h>
#include <macro.h>
#include <window.h>
#include <vfs.h>

extern uint32_t WINDOW_COLOR;
extern uint32_t WINDOW_TITLE_COLOR;
extern uint32_t WINDOW_TITLE_FCOLOR;
extern uint32_t WINDOW_CLOSE_BUTTON_COLOR;
extern uint32_t WINDOW_CLOSE_BUTTON_BACKCOLOR;

int strhex2num(char *s)
{
    int result=0;
    if(strncmp(s,"0x",2)!=0)
    {
        return -1;
    }
    int i,n;
    for(i=2;s[i]!=0;i++)
    {
    	switch(s[i])
    	{
    		case '0':n=0;break;
    		case '1':n=1;break;
    		case '2':n=2;break;
    		case '3':n=3;break;
    		case '4':n=4;break;
    		case '5':n=5;break;
    		case '6':n=6;break;
    		case '7':n=7;break;
    		case '8':n=8;break;
    		case '9':n=9;break;
    		case 'a':n=10;break;
    		case 'b':n=11;break;
    		case 'c':n=12;break;
    		case 'd':n=13;break;
    		case 'e':n=14;break;
    		case 'f':n=15;break;
    		case 'A':n=10;break;
    		case 'B':n=11;break;
    		case 'C':n=12;break;
    		case 'D':n=13;break;
    		case 'E':n=14;break;
    		case 'F':n=15;break;
    		default:return -1;
    	}
    	result=result*16+n;
    }
    return result;
}

int load_wallpaper(uint32_t *vram,int x,int y,char *wallpaper_name);

int theme_set(char *themename,uint32_t *buf)
{
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    char wtbc[30],wtfc[30],wbbc[30],wbfc[30],wp[30];
    if(vfs_open(themename)==NULL)
    {
        return -1;
    }
    if(read_ini(themename,"window","window_title_bc",wtbc)!=0)
    {
        return -1;
    }
    if(read_ini(themename,"window","window_title_fc",wtfc)!=0)
    {
        return -1;
    }
    if(read_ini(themename,"window","window_close_bc",wbbc)!=0)
    {
        return -1;
    }
    if(read_ini(themename,"window","window_close_fc",wbfc)!=0)
    {
        return -1;
    }
    
    int n;
    n=strhex2num(wtbc);
    if(n==-1)
    {
        return -1;
    }
    WINDOW_TITLE_COLOR=n;
    
    n=strhex2num(wtfc);
    if(n==-1)
    {
        return -1;
    }
    WINDOW_TITLE_FCOLOR=n;
    
    n=strhex2num(wbbc);
    if(n==-1)
    {
        return -1;
    }
    WINDOW_CLOSE_BUTTON_BACKCOLOR=n;
    
    n=strhex2num(wbfc);
    if(n==-1)
    {
        return -1;
    }
    WINDOW_CLOSE_BUTTON_COLOR=n;

    if(read_ini(themename,"wallpaper","wallpaper",wp)!=0)
    {
        return -1;
    }
    load_wallpaper(buf,binfo->scrnx,binfo->scrny,wp);
    window_redraw();
    return 0;
}

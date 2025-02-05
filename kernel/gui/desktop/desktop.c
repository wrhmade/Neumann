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
#include <stdint.h>
#include <jpeg.h>
#include <bmp.h>
#include <fat16.h>
#include <mm.h>
#include <ini.h>
#include <string.h>
#include <task.h>
#include <theme.h>
#include <sheet.h>

extern sheet_t *global_sht_back;
extern uint32_t *global_buf_back;

void desktop_reload(char *themename)
{
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
	const uint32_t xsize=binfo->scrnx,ysize=binfo->scrny;
	char *value=(char *)malloc(sizeof(char)*10);
    //boxfill(global_buf_back,xsize,0,0,xsize-1,ysize-1,DESKTOP_BACKCOLOR);
	//char wallpaper[30];
	int returnvalue=theme_set(themename,global_buf_back);
	
	
	read_ini("neumann.ini","wallpaper","wallpaper",value);
	if(strcmp(value,"true")==0)
	{
		if(returnvalue==-1)
		{
			read_ini("neumann.ini","wallpaper","wallpaper_filename",value);
			load_wallpaper(global_buf_back,binfo->scrnx,binfo->scrny,value);	
		}

		
	}
	free(value);
	
    putstr_ascii(global_buf_back,xsize,xsize-15*8,ysize-77,ARGB(255,0,0,0),"Neumann操作系统");
    putstr_ascii(global_buf_back,xsize,xsize-18*8,ysize-61,ARGB(255,0,0,0),"版本号0.8[Beta 6]");
    putstr_ascii(global_buf_back,xsize,xsize-28*8,ysize-45,ARGB(255,0,0,0),"注意:这是测试版,可能会不稳定");
	sheet_refresh(global_sht_back,0,0,binfo->scrnx-1,binfo->scrny-1);
}

void init_desktop(uint32_t *buf,uint32_t xsize,uint32_t ysize)
{
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
	char *value=(char *)malloc(sizeof(char)*10);
    boxfill(buf,xsize,0,0,xsize-1,ysize-1,DESKTOP_BACKCOLOR);
	//char wallpaper[30];
	if(read_ini("neumann.ini","theme","default",value)!=0)
	{
		strcpy(value,"default.tme");
	}
	int returnvalue=theme_set(value,buf);
	
	
	read_ini("neumann.ini","wallpaper","wallpaper",value);
	if(strcmp(value,"true")==0)
	{
		if(returnvalue==-1)
		{
			read_ini("neumann.ini","wallpaper","wallpaper_filename",value);
			load_wallpaper(buf,binfo->scrnx,binfo->scrny,value);	
		}

		
	}
	free(value);
	
    putstr_ascii(buf,xsize,xsize-15*8,ysize-77,ARGB(255,0,0,0),"Neumann操作系统");
    putstr_ascii(buf,xsize,xsize-18*8,ysize-61,ARGB(255,0,0,0),"版本号0.8[Beta 6]");
    putstr_ascii(buf,xsize,xsize-28*8,ysize-45,ARGB(255,0,0,0),"注意:这是测试版,可能会不稳定");


}

void draw_mouse(uint32_t *buf_mouse)
{
    int x,y;
    static char cursor[24][24] = {
		"O.......................", 
		"OO......................", 
		"O*O.....................", 
		"O**O....................", 
		"O***O...................", 
		"O****O..................", 
		"O*****O.................", 
		"O******O................", 
		"O*******O...............", 
		"O********O..............", 
		"O*********O.............", 
		"O******OOOOO............", 
		"O***O**O................", 
		"O**OO**O................", 
		"O*O..O**O...............", 
		"OO...O**O...............", 
		"O.....O**O..............", 
		"......O**O..............", 
		".......O**O.............", 
		".......O**O.............", 
		"........OO..............", 
		"........................", 
		"........................", 
		"........................"
	};
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 24; x++) {
			if (cursor[y][x] == '*') {
				buf_mouse[y * 24 + x] = 0x000000;
			}
			if (cursor[y][x] == 'O') {
				buf_mouse[y * 24 + x] = 0xFFFFFF;
			}
			if (cursor[y][x] == '.') {
				buf_mouse[y * 24 + x] = DESKTOP_BACKCOLOR;
			}
		}
	}
}

int load_wallpaper(uint32_t *vram,int x,int y,char *wallpaper_name)
{
	fileinfo_t *finfo;
	char *buf;
	struct DLL_STRPICENV *env;
	struct RGB *picbuf;
	int info[4],i,j,x0,y0;
	uint8_t r,g,b;
	finfo=(fileinfo_t *)malloc(sizeof(fileinfo_t));
	if(fat16_open_file(finfo,wallpaper_name)!=0)
	{
		free(finfo);
		return -1;
	}
	buf=(char *)malloc(sizeof(char)*(finfo->size+5));
	fat16_read_file(finfo,buf);
	env=(struct DLL_STRPICENV *)malloc(sizeof(struct DLL_STRPICENV));
	if(info_BMP(env,info,finfo->size,buf)==0)
	{
		if(info_JPEG(env,info,finfo->size,buf)==0)
		{
			free(finfo);
			free(buf);
			free(env);
			return -1;
		}
	}
	picbuf=(struct RGB *)malloc(sizeof(struct RGB)*info[2]*info[3]);
	if(info[0]==1)
	{
		decode0_BMP(env,finfo->size,buf,4,(unsigned char *)picbuf,0);
	}
	else
	{
		decode0_JPEG(env,finfo->size,buf,4,(unsigned char *)picbuf,0);
	}
	
	x0 = (int) ((x - info[2]) / 2);
	y0 = (int) ((y - info[3]) / 2);
	

	for(i=0;i<info[3];i++)
	{
		for(j=0;j<info[2];j++)
		{
			if(x0+j>=0 && x0+j<x && y0+i>=0 && y0+i<y)
			{
				r=picbuf[i*info[2]+j].r;
				g=picbuf[i*info[2]+j].g;
				b=picbuf[i*info[2]+j].b;
				vram[(y0 + i) * x + (x0 + j)]=ARGB(255,r,g,b);
			}
		}
	}
	free(picbuf);
	free(env);
	free(buf);
	free(finfo);
	//free(filename);
}
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
#include <mm.h>
#include <ini.h>
#include <string.h>
#include <task.h>
#include <theme.h>
#include <sheet.h>
#include <krnlcons.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>

#define STBI_MALLOC(sz) kmalloc(sz)
#define STBI_REALLOC(p,newsz) krealloc(p,newsz)
#define STBI_FREE(p) kfree(p)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_THREAD_LOCALS
#include <stb_image.h>


extern sheet_t *global_sht_back;
extern uint32_t *global_buf_back;

struct RGBA
{
	uint8_t r,g,b,a;
};

void desktop_reload(char *themename)
{
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
	const uint32_t xsize=binfo->scrnx,ysize=binfo->scrny;
	char *value=(char *)kmalloc(sizeof(char)*10);
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
	kfree(value);
	
    putstr_ascii(global_buf_back,xsize,xsize-15*8,ysize-77,ARGB(255,0,0,0),"Neumann操作系统");
    putstr_ascii(global_buf_back,xsize,xsize-18*8,ysize-61,ARGB(255,0,0,0),"版本号0.8[Beta 6]");
    putstr_ascii(global_buf_back,xsize,xsize-28*8,ysize-45,ARGB(255,0,0,0),"注意:这是测试版,可能会不稳定");
	sheet_refresh(global_sht_back,0,0,binfo->scrnx-1,binfo->scrny-1);
}

void init_desktop(uint32_t *buf,uint32_t xsize,uint32_t ysize)
{
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
	char *value=(char *)kmalloc(sizeof(char)*50);
    boxfill(buf,xsize,0,0,xsize-1,ysize-1,DESKTOP_BACKCOLOR);
	//char wallpaper[30];
	if(read_ini("/config/neumann.ini","theme","default",value)!=0)
	{
		strcpy(value,"default.tme");
	}
	int returnvalue=theme_set(value,buf);
	
	read_ini("/config/neumann.ini","wallpaper","wallpaper",value);
	if(strcmp(value,"true")==0)
	{
		if(returnvalue==-1)
		{
			read_ini("/config/neumann.ini","wallpaper","wallpaper_filename",value);
			load_wallpaper(buf,binfo->scrnx,binfo->scrny,value);	
		}

		
	}
	kfree(value);
	
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

void convert_ABGR_to_ARGB(uint32_t* bitmap, size_t num_pixels) {
  for (size_t i = 0; i < num_pixels; ++i) {
    uint32_t pixel = bitmap[i];
    uint8_t alpha = (pixel >> 24) & 0xFF;
    uint8_t red = (pixel >> 16) & 0xFF;
    uint8_t green = (pixel >> 8) & 0xFF;
    uint8_t blue = pixel & 0xFF;
    bitmap[i] = (alpha << 24) | (blue << 16) | (green << 8) | red;
  }
}

int load_wallpaper(uint32_t *vram,int x,int y,char *wallpaper_name)
{
	vfs_node_t node;
	char *buf;
	int info[4],i,j,x0,y0,x1,y1;
	uint8_t r,g,b,a;
	struct RGBA *img;
	node=vfs_open(wallpaper_name);
	if(!node)
	{
		return -1;
	}
	buf=(char *)kmalloc(node->size+5);
	vfs_read(node,buf,0,node->size);
	
	int width,height,bpp;
	char s[200];
	unsigned char *idata=stbi_load_from_memory(buf,node->size,&width,&height,&bpp,4);

	

	


	if(!idata)
	{
		const char *error=stbi_failure_reason();
		
		sprintf(s,"stb_image.h:Load error:%s\n",error);
		krnlcons_putstr_color(s,0xFFFFFF,0);
		kfree(buf);
		return -1;

	}
	img=(struct RGBA *)idata;

	// krnlcons_putstr("stb_image.h:Success.\n");
	// sprintf(s,"stb_image.h:width:%d,height:%d\n",width,height);
	// krnlcons_putstr(s);

	// sleep(100);
	// 

	// for(i=0;i<width*height;i++)
	// {
	// 	sprintf(s,"stb_image.h:Pixel %d: %d %d %d %d\n", i, img[i].r, img[i].g, img[i].b, img[i].a);
	// 	krnlcons_putstr(s);
	// 	sleep(50);
	// }
	

	// for(;;);




	x0 = (int) ((x - width) / 2);
	y0 = (int) ((y - height) / 2);
	

	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			if(x0+j>=0 && x0+j<x && y0+i>=0 && y0+i<y)
			{
				r=img[i*width+j].r;
				g=img[i*width+j].g;
				b=img[i*width+j].b;
				vram[(y0 + i) * x + (x0 + j)]=ARGB(255,r,g,b);
			}
		}
	}

	//kfree(picbuf);
	//kfree(env);
	kfree(buf);
	stbi_image_free(idata);
	//kfree(filename);
}
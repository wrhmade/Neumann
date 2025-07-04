/*
graphic.c 
图形操作
Copyright W24 Studio 
*/

#include <graphic.h>
#include <stdint.h>
#include <task.h>
#include <macro.h>
#include <binfo.h>
#include <mm.h>
#include <stddef.h>
#include <dbuffer.h>

uint16_t LCD_AlphaBlend(uint32_t foreground_color,uint32_t background_color,uint8_t alpha)
{
	uint16_t r=0,g=0,b=0;
	if((foreground_color==0xffffff)&&(background_color==0)){	//默认的前景和背景色，不做alpha计算
		r=alpha;
		g=alpha;
		b=alpha;
	}
	else{
		uint8_t *fg = (uint8_t *)&foreground_color;
		uint8_t *bg = (uint8_t *)&background_color;
			
		b = ((int)(*fg * alpha) + (int)*bg * (256 - alpha))>>8;
		fg++;bg++;
		g = ((int)(*fg * alpha) + (int)*bg * (256 - alpha))>>8;
		fg++;bg++;
		r = ((int)(*fg * alpha) + (int)*bg * (256 - alpha))>>8;
	}
	uint16_t temp= (((b >>3) & 0x1f)<<0)|(((g>>2) & 0x3f) << 5) |(((r >>3) & 0x1f) <<11);
	return temp;
}

void boxfill(uint32_t *vram,uint16_t xsize,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint32_t c)
{
    int x,y;
	for(y = y1; y <= y2; y++)
	{
		for(x = x1; x <= x2; x++)
		{
			vram[y*xsize+x]=c;
		}
	}
}

void putfont(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *font)
{
	int i;
	uint32_t *p;
    char d; /* data */
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

void putstr_ascii(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, unsigned char *s)
{
	task_t *task=task_now();
    putstr_ascii_lmode(vram,xsize,x,y,c,s,task->langmode);
}

void putstr_ascii_lmode(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, unsigned char *s,int langmode)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    extern char asciifnt[4096];
	char *font;
    int i,k,t;
	task_t *task=task_now();
	if(langmode==0 || (langmode==1 && binfo->hzk16==NULL) || (langmode==2 && binfo->hzk16f==NULL))//英文模式或者中文模式但时没有中文字库时
	{
    	for(i=0;s[i]!=0;i++)
    	{
    	    putfont(vram,xsize,x,y,c,asciifnt+s[i]*16);
    	    x+=8;
    	}
	}
	else if(langmode==1 && binfo->hzk16!=NULL)//简体中文模式且有中文字库时
	{
		for(i=0;s[i]!=0;i++)
    	{

    	    if(task->langbyte==0)
			{
				if(0xa1<=s[i] && s[i]<=0xfe)
				{
					task->langbyte=s[i];
				}
				else
				{
					putfont(vram,xsize,x,y,c,asciifnt+s[i]*16);
				}
			}
			else
			{
				k=task->langbyte-0xa1;
				t=s[i]-0xa1;
				task->langbyte=0;
				font = binfo->hzk16 + (k * 94 + t) * 32;
				putfont_gb2312(vram,xsize,x-8,y,c,font,font+16);
			}
			x+=8;
    	}
	}
	else if(langmode==2 && binfo->hzk16f!=NULL)//繁体中文模式且有中文字库时
	{
		for(i=0;s[i]!=0;i++)
    	{

    	    if(task->langbyte==0)
			{
				if(0xa1<=s[i] && s[i]<=0xfe)
				{
					task->langbyte=s[i];
				}
				else
				{
					putfont(vram,xsize,x,y,c,asciifnt+s[i]*16);
				}
			}
			else
			{
				k=task->langbyte-0xa1;
				t=s[i]-0xa1;
				task->langbyte=0;
				font = binfo->hzk16f + (k * 94 + t) * 32;
				putfont_gb2312(vram,xsize,x-8,y,c,font,font+16);
			}
			x+=8;
    	}
	}
}

void putfont_gb2312(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,char *font1,char *font2)
{
	char New1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	char New2[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int i=0,j=0;
	while(i<8)
	{
		New1[i]=font1[j];
		i++;
		j+=2;
	}
	j=0;
	while(i<16)
	{
		New1[i]=font2[j];
		i++;
		j+=2;
	}
	i=0;j=1;
	while(i<8)
	{
		New2[i]=font1[j];
		i++;
		j+=2;
	}
	j=1;
	while(i<16)
	{
		New2[i]=font2[j];
		i++;
		j+=2;
	}
	putfont(vram,xsize,x,y,c,New1);
	putfont(vram,xsize,x+8,y,c,New2);
																	
}

void putblock(uint32_t *vram, int vxsize, int pxsize,int pysize, int px0, int py0, uint32_t *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}
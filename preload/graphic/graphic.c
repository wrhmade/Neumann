/*
graphic.c 
图形操作
Copyright W24 Studio 
*/

#include <graphic.h>
#include <stdint.h>
#include <macro.h>
#include <binfo.h>
#include <stddef.h>


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

void putstr_ascii(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c,char *s)
{
    extern char asciifnt[4096];
	int i;
	for(i=0;s[i]!=0;i++)
    {
        putfont(vram,xsize,x,y,c,asciifnt+s[i]*16);
        x+=8;
    }
}

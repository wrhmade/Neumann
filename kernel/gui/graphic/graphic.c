/*
graphic.c 
图形操作
Copyright W24 Studio 
*/

#include <graphic.h>
#include <stdint.h>


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
    int i,j;
    for(i=y1;i<=y2;i++)
    {
        for(j=x1;j<=x2;j++)
        {
            vram[i*xsize+j]=c;
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

void putstr_ascii(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *s)
{
    extern char asciifnt[4096];
    int i;
    for(i=0;s[i]!=0;i++)
    {
        putfont(vram,xsize,x,y,c,asciifnt+s[i]*16);
        x+=8;
    }
}

void putstr_ascii_bc(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, uint32_t bc,char *s)
{
    extern char asciifnt[4096];
    int i;
    for(i=0;s[i]!=0;i++)
    {
		boxfill(vram,xsize,x,y,strlen(s)*8,y+15,bc);
		putfont(vram,xsize,x,y,c,asciifnt+s[i]*16);
        x+=8;
    }
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
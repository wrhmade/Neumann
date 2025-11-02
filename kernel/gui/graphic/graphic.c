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

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)  ((void)(u),kmalloc(x))
#define STBTT_free(x,u)    ((void)(u),kfree(x))
#include <stb_truetype.h>
#include <vfs.h>

#define ALPHABLEND_SUB(fc,bc,a) (fc*a+bc*(255-a)+128)>>8

uint32_t alphablend(uint32_t fc,uint32_t bc,uint8_t a)
{
	uint8_t fr,fg,fb;
	uint8_t br,bg,bb;
	uint8_t r,g,b;
	fr=(fc>>16)&0xFF;
	fg=(fc>>8)&0xFF;
	fb=fc&0xFF;

	br=(bc>>16)&0xFF;
	bg=(bc>>8)&0xFF;
	bb=bc&0xFF;

	r=ALPHABLEND_SUB(fr,br,a);
	g=ALPHABLEND_SUB(fg,bg,a);
	b=ALPHABLEND_SUB(fb,bb,a);

	return ARGB(255,r,g,b);
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

void putstr_ascii(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *s)
{
	task_t *task=task_now();
    putstr_ascii_lmode(vram,xsize,x,y,c,s,task->langmode);
}

void putstr_ascii_lmode(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *s,int langmode)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    extern char asciifnt[4096];
	unsigned char *p=(unsigned char *)s;
	char *font;
    int i,k,t;
	task_t *task=task_now();
	if(langmode==0 || (langmode==1 && binfo->hzk16==NULL) || (langmode==2 && binfo->hzk16f==NULL))//英文模式或者中文模式但是没有中文字库时
	{
    	for(i=0;p[i]!=0;i++)
    	{
    	    putfont(vram,xsize,x,y,c,asciifnt+p[i]*16);
    	    x+=8;
    	}
	}
	else if(langmode==1 && binfo->hzk16!=NULL)//简体中文模式且有中文字库时
	{
		for(i=0;p[i]!=0;i++)
    	{

    	    if(task->langbyte==0)
			{
				if(0xa1<=p[i] && p[i]<=0xfe)
				{
					task->langbyte=p[i];
				}
				else
				{
					putfont(vram,xsize,x,y,c,asciifnt+p[i]*16);
				}
			}
			else
			{
				k=task->langbyte-0xa1;
				t=p[i]-0xa1;
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
				if(0xa1<=p[i] && p[i]<=0xfe)
				{
					task->langbyte=p[i];
				}
				else
				{
					putfont(vram,xsize,x,y,c,asciifnt+p[i]*16);
				}
			}
			else
			{
				k=task->langbyte-0xa1;
				t=p[i]-0xa1;
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

void putfont_ttf(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,uint32_t bc,const unsigned char *ttf_buffer,int chr,uint32_t size)
{
    if(ttf_buffer == 0)
    {
        return;
    }
    
    stbtt_fontinfo info;
    if(stbtt_InitFont(&info, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0)))
    {
        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        float scale = stbtt_ScaleForPixelHeight(&info, size);
        int width, height, xoff, yoff;
        uint8_t *bitmap = stbtt_GetCodepointBitmap(&info, 0, scale, chr, &width, &height, &xoff, &yoff);
        if(bitmap == 0)
        {
            return;
        }
        int char_height = (int)((ascent - descent) * scale);
        int y_offset = y - char_height + (int)(ascent * scale);
        for(int i = 0; i < height; i++)
        {
            for(int j = 0; j < width; j++)
            {
                int current_y = y_offset + yoff + i;
                if(current_y >= 0 && current_y < xsize) {
                    vram[current_y * xsize + (x + j)] = alphablend(c, bc, bitmap[i * width + j]);
                }
            }
        }

        stbtt_FreeBitmap(bitmap, NULL);
    }
}



void putstr_ttf(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,uint32_t bc,const unsigned char *ttf_buffer,char *s,uint32_t size)
{
	int current_x=x;
	int width,height;
	stbtt_fontinfo info;
	if(stbtt_InitFont(&info,ttf_buffer,stbtt_GetFontOffsetForIndex(ttf_buffer,0)))
	{
		for(int i=0;s[i]!=0;i++)
		{
			putfont_ttf(vram,xsize,current_x,y,c,bc,ttf_buffer,s[i],size);
			if(s[i]==' ')
			{
        		int advance, left_side_bearing;
        		stbtt_GetCodepointHMetrics(&info, ' ', &advance, &left_side_bearing);
        		float scale = stbtt_ScaleForPixelHeight(&info, size);
        		int space_width = (int)(advance * scale);
				current_x+=space_width;
			}
			else
			{
				uint8_t *bitmap=stbtt_GetCodepointBitmap(&info,0,stbtt_ScaleForPixelHeight(&info,size),s[i],&width,&height,0,0);
				stbtt_FreeBitmap(bitmap,NULL);
				current_x+=width;
			}
		}
	}
}

void putstr_ttf_file(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,uint32_t bc,char *ttf_filename,char *s,uint32_t size,int instead)
{
	vfs_node_t node;
	node=vfs_open(ttf_filename);
	if(node==0)
	{
		if(instead)
		{
			putstr_ascii(vram,xsize,x,y-16,c,s);	
		}
		return;
	}
	else
	{
		char *buffer=kmalloc(node->size);
		vfs_read(node,buffer,0,node->size);
		putstr_ttf(vram,xsize,x,y,c,bc,(const unsigned char *)buffer,s,size);
		kfree(buffer);
	}
}

int hzk16_load(const char *filename)
{
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
	vfs_node_t node=vfs_open(filename);

	if(node==0)
	{
		return -1;
	}

	//为了保证安全，先分配再释放

	char *old=binfo->hzk16;
	binfo->hzk16=kmalloc(node->size+5);
	vfs_read(node,binfo->hzk16,0,node->size);
	kfree(old);

	return 0;
}

int hzk16f_load(const char *filename)
{
	struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
	vfs_node_t node=vfs_open(filename);

	if(node==0)
	{
		return -1;
	}

	//为了保证安全，先分配再释放

	char *old=binfo->hzk16f;
	binfo->hzk16f=kmalloc(node->size+5);
	vfs_read(node,binfo->hzk16f,0,node->size);
	kfree(old);

	return 0;
}
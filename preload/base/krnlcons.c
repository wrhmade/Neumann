/*
krnlcons.c
内核控制台界面
Copyright W24 Studio 
*/

#include <krnlcons.h>
#include <binfo.h>
#include <graphic.h>
#include <macro.h>
#include <string.h>

static int krnlcons_cx,krnlcons_cy;
struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;

int BACKCOLOR=0x000000;
int FORECOLOR=0xAAAAAA;

void krnlcons_display()
{
	krnlcons_cleanscreen();
	
}

void krnlcons_showcur()
{
	for(int y=0;y<16;y++)
	{
		for(int x=0;x<8;x++)
		{
			if(binfo->vram[(krnlcons_cy+y)*binfo->scrnx+(krnlcons_cx+x)]==0x000000)
			{
				binfo->vram[(krnlcons_cy+y)*binfo->scrnx+(krnlcons_cx+x)]=FORECOLOR-binfo->vram[(krnlcons_cy+y)*binfo->scrnx+(krnlcons_cx+x)];
			}
			else
			{
				binfo->vram[(krnlcons_cy+y)*binfo->scrnx+(krnlcons_cx+x)]=0xFFFFFF-binfo->vram[(krnlcons_cy+y)*binfo->scrnx+(krnlcons_cx+x)];
			}
		}
	}
	
	//boxfill(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,krnlcons_cx+7,krnlcons_cy+15,FORECOLOR);
}

void krnlcons_cleanscreen()
{
	boxfill(binfo->vram,binfo->scrnx,0,0,binfo->scrnx-1,binfo->scrny-1,BACKCOLOR);
	krnlcons_cx=krnlcons_cy=0;
	krnlcons_showcur();
	
}

void krnlcons_newline()
{
	int x,y;
	if(krnlcons_cy>=binfo->scrny-16)
	{
		for(y=0;y<binfo->scrny-16;y++)
		{
			for(x=0;x<binfo->scrnx;x++)
			{
				binfo->vram[x + y * binfo->scrnx] = binfo->vram[x + (y + 16) * binfo->scrnx];
			}
		}
		boxfill(binfo->vram,binfo->scrnx,0,binfo->scrny-16,binfo->scrnx-1,binfo->scrny-1,BACKCOLOR);
		krnlcons_cx=0;
	}
	else
	{
		krnlcons_cy+=16;
		krnlcons_cx=0;
	}
	
}

void krnlcons_putchar_color(char c,int cc,int bc)
{
	char s[2];
	if(c=='\n')
	{
		boxfill(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,krnlcons_cx+7,krnlcons_cy+15,BACKCOLOR);
		krnlcons_newline();
	}
	else if(c=='\t')
	{
		boxfill(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,krnlcons_cx+7,krnlcons_cy+15,BACKCOLOR);
        krnlcons_cx=((krnlcons_cx/8 + 4) & ~(4 - 1))*8;
	}
	else if(c>=0x20 && c<=0xff)
	{
		boxfill(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,krnlcons_cx+7,krnlcons_cy+15,bc);
		s[0]=c;
		s[1]=0;
		putstr_ascii(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,cc,s);
		krnlcons_cx+=8;
	}

	if(krnlcons_cx>=binfo->scrnx-1)
	{
		krnlcons_newline();
	}
	krnlcons_showcur();
	
}

void krnlcons_putchar_color_nomove(char c,int cc,int bc)
{
	char s[2];
	if(c=='\n')
	{
		boxfill(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,krnlcons_cx+7,krnlcons_cy+15,BACKCOLOR);
		krnlcons_newline();
	}
	else if(c=='\t')
	{
        boxfill(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,krnlcons_cx+7,krnlcons_cy+15,BACKCOLOR);
        krnlcons_cx=((krnlcons_cx/8 + 4) & ~(4 - 1))*8;
	}
	else if(c>=0x20 && c<=0xff)
	{
		boxfill(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,krnlcons_cx+7,krnlcons_cy+15,bc);
		s[0]=c;
		s[1]=0;
		putstr_ascii(binfo->vram,binfo->scrnx,krnlcons_cx,krnlcons_cy,cc,s);
	}

	if(krnlcons_cx>=binfo->scrnx-1)
	{
		krnlcons_newline();
	}
	krnlcons_showcur();
	
}

void krnlcons_putchar(char c)
{
	krnlcons_putchar_color(c,FORECOLOR,BACKCOLOR);
}

void krnlcons_putchar_nomove(char c)
{
	krnlcons_putchar_color_nomove(c,FORECOLOR,BACKCOLOR);
}

void krnlcons_putstr(char *s)
{
	int i;
	for(i=0;s[i]!=0;i++)
	{
		krnlcons_putchar(s[i]);
	}
}

void krnlcons_putstr_color(char *s,int cc,int cb)
{
	int i;
	for(i=0;s[i]!=0;i++)
	{
		krnlcons_putchar_color(s[i],cc,cb);
	}
}

void krnlcons_change_backcolor(int c)
{
	BACKCOLOR=c;
	
}

void krnlcons_change_forecolor(int c)
{
	FORECOLOR=c;
	
}
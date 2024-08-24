/*
main.c
进入C语言后的第一个程序
Copyright W24 Studio 
*/

#include <nasmfunc.h>
#include <binfo.h> 
#include <macro.h>
#include <sysset.h>
#include <graphic.h>
#include <desktop.h>
#include <gdtidt.h>
#include <mouse.h>
#include <keyboard.h>
#include <fifo.h>
#include <mm.h>
#include <sheet.h>
#include <window.h>
#include <timer.h>
#include <task.h>
#include <stddef.h>
#include <console.h>

extern fifo_t decoded_key;
extern fifo_t mouse_fifo;
mdec_t mdec;

sheet_t *global_shtctl;
int process=0;

#define PROCESS_COLOR 0xFF0000
#define PROCESS_BACKCOLOR DESKTOP_BACKCOLOR
#define PROCESS_SUM 9


void process_forward(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int _process=binfo->scrnx/2-320/2+60+process;
	boxfill(binfo->vram,binfo->scrnx,_process,binfo->scrny/2-240/2+200,_process+(200/PROCESS_SUM),binfo->scrny/2-240/2+220,PROCESS_COLOR);
	process+=200/PROCESS_SUM;
}

void taskb_main(void)
{
	task_t *task=task_now();
	char s[10];
	int tick=0;
    
	for(;;)
	{
		sprintf(s,"%09u",tick);
		putstr_ascii_sheet(task->window->sheet,0,18,0x000000,0xFFFFFF,s);
		tick++;
	}
}


void krnlc_main(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int i,j;
	int mouse_x,mouse_y,mmx=-1,mmy=-1,mmx2=0,x,y;
	int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
	uint32_t memtotal;
	process=0;
	char s[40];
	shtctl_t *shtctl;
	sheet_t *sht_back;
	uint32_t *buf_back;
	sheet_t *sht_mouse;
	uint32_t *buf_mouse;
	sheet_t *sht;
	sheet_t *keywin;
	console_t *console;
	
	boxfill(binfo->vram,binfo->scrnx,0,0,binfo->scrnx,binfo->scrny,PROCESS_BACKCOLOR);
	boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-320/2-5,binfo->scrny/2-240/2-5,binfo->scrnx/2+320/2-5,binfo->scrny/2+240/2-5,0x848484);
	boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-320/2,binfo->scrny/2-240/2,binfo->scrnx/2+320/2,binfo->scrny/2+240/2,0xFFFFFF);
	boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-320/2+58,binfo->scrny/2-240/2+198,binfo->scrnx/2-320/2+262,binfo->scrny/2-240/2+222,0x000000);
	boxfill(binfo->vram,binfo->scrnx,binfo->scrnx/2-320/2+60,binfo->scrny/2-240/2+200,binfo->scrnx/2-320/2+260,binfo->scrny/2-240/2+220,0xC6C6C6);
	putstr_ascii(binfo->vram,binfo->scrnx,binfo->scrnx/2-16*8/2,binfo->scrny/2-240/2+180,0,"Starting Neumann");

	// boxfill(binfo->vram,binfo->scrnx,20,20,300,300,0xFF0000);
	// boxfill(binfo->vram,binfo->scrnx,40,40,320,320,0x00FF00);
	// boxfill(binfo->vram,binfo->scrnx,60,60,340,340,0x0000FF);
	
	init_gdtidt();process_forward();


	asm_sti();process_forward();


	init_keyboard();process_forward();


	init_ps2mouse();process_forward();

	memtotal=init_mem();process_forward();
	sprintf(s,"memtotal=%uMB",memtotal/1024/1024);
	binfo->memtotal=memtotal;
	putstr_ascii(binfo->vram,binfo->scrnx,0,0,0x000000,s);


	shtctl = shtctl_init(binfo->vram, binfo->scrnx, binfo->scrny);
	global_shtctl=shtctl;
	process_forward();
	sht_back  = sheet_alloc(shtctl);
    buf_back  = (uint32_t*) malloc(sizeof(uint32_t)*binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny,-1);
	

	process_forward();

	task_t *task_a=task_init();
	process_forward();

	init_timer(100);
	process_forward();

	

	init_desktop(buf_back,binfo->scrnx,binfo->scrny);

	
	
	

	
	

	buf_mouse=(uint32_t *)malloc(sizeof(uint32_t)*24*24);
	draw_mouse(buf_mouse);
	

	sht_mouse=sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse,buf_mouse,24,24,DESKTOP_BACKCOLOR);
	
	
	
	mouse_x=binfo->scrnx/2;
	mouse_y=binfo->scrny/2;

	sheet_slide(sht_back,  0,  0);
	sheet_updown(sht_back,  0);
	sheet_slide(sht_mouse,  mouse_x,  mouse_y);
	sheet_updown(sht_mouse,  1);

	//putblock(binfo->vram,binfo->scrnx,24,24,mouse_x,mouse_y,mouse,24);
	uint8_t mouse_data,keyboard_data;


	// window_t *window=create_window("The First Window",200,200,-1);
	// show_window(window);
	// move_window(window,binfo->scrnx/2,binfo->scrny/2);
	console=open_console();
	keywin=console->window->sheet;


	int _free,free;
	for(;;)
	{

		if(fifo_status(&mouse_fifo)>0)
		{
			mouse_data=fifo_get(&mouse_fifo);
			if(mouse_decode(&mdec,mouse_data)!=0)
			{
				//boxfill(binfo->vram,binfo->scrnx,mouse_x,mouse_y,mouse_x+23,mouse_y+23,DESKTOP_BACKCOLOR);
				mouse_x+=mdec.x;
				mouse_y+=mdec.y;
				if (mouse_x < 0) {
					mouse_x = 0;
				}
				if (mouse_y < 0) {
					mouse_y = 0;
				}
				if (mouse_x > binfo->scrnx-1)
				{
					mouse_x = binfo->scrnx-1;
				}
				if (mouse_y > binfo->scrny-1)
				{
					mouse_y = binfo->scrny-1;
				}
				new_mx=mouse_x;
				new_my=mouse_y;
				if((mdec.btn & 0x01)!=0)
				{
					if(mmx<0)
					{
						for(i=shtctl->top-1;i>0;i--)
						{
							sht = shtctl->sheets[i];
							x=mouse_x-sht->vx0;
							y=mouse_y-sht->vy0;
							if(0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize)
							{
								sheet_updown(sht,shtctl->top-1);
								keywin=sht;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < 18) {
    		                        mmx = mouse_x;
    		                        mmy = mouse_y;
    		                        mmx2 = sht->vx0;
    		                        new_wy = sht->vy0;
    		                    }
								if(sht->bxsize-17 <= x && x<= sht->bxsize-1 && 1<=y && y<=17)
								{
									if(sht->window->isconsole)
									{
										close_console(sht->window->console);
									}
									else
									{
										if(sht->window->task!=NULL)
										{
											task_remove(sht->window->task);
										}
										close_window(sht->window);
									}
								}
								break;
							}
						}
					}
					else
					{
						x = mouse_x - mmx;
    		            y = mouse_y - mmy;
    		            new_wx = (mmx2 + x + 2) & ~3;
    		            new_wy = new_wy + y;
    		            mmy = mouse_y;
					}
				}
				else
				{
					mmx = -1; /* 记录鼠标没有按下左键 */
    		        /* 将窗口移动到鼠标移动位置处,并标识窗口已移动过一次 */
    		        if (new_wx != 0x7fffffff) {
    		            move_window(sht->window, new_wx, new_wy);
    		            new_wx = 0x7fffffff;
    		        }
				}
			}
		}
		if(fifo_status(&decoded_key)>0)
		{
			keyboard_data=fifo_get(&decoded_key);
			fifo_put(&keywin->window->task->fifo,keyboard_data+256);
		}
		

		if(fifo_status(&decoded_key)==0 && fifo_status(&mouse_fifo)==0)
		{
			if (new_mx >= 0) {
                sheet_slide(sht_mouse, new_mx, new_my);
                new_mx = -1;
            } else if (new_wx != 0x7fffffff) {
                sheet_slide(sht, new_wx, new_wy);
                new_wx = 0x7fffffff;
            }
		}
	}
}

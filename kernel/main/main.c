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

extern fifo_t decoded_key;
extern fifo_t mouse_fifo;
mdec_t mdec;

int process=0;

#define PROCESS_COLOR 0xFF0000
#define PROCESS_BACKCOLOR DESKTOP_BACKCOLOR
#define PROCESS_SUM 7

void process_forward(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int _process=binfo->scrnx/2-320/2+60+process;
	boxfill(binfo->vram,binfo->scrnx,_process,binfo->scrny/2-240/2+200,_process+(200/PROCESS_SUM),binfo->scrny/2-240/2+220,PROCESS_COLOR);
	process+=200/PROCESS_SUM;
}

void krnlc_main(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	int i,j;
	uint32_t memtotal;
	process=0;
	char s[40];
	shtctl_t *shtctl;
	sheet_t *sht_back;
	uint32_t *buf_back;
	sheet_t *sht_mouse;
	uint32_t *buf_mouse;
	
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
	putstr_ascii(binfo->vram,binfo->scrnx,0,0,0x000000,s);


	shtctl = shtctl_init(binfo->vram, binfo->scrnx, binfo->scrny);
	process_forward();
	sht_back  = sheet_alloc(shtctl);
    buf_back  = (uint32_t*) malloc(sizeof(uint32_t)*binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny,-1);
	

	process_forward();

	init_desktop(buf_back,binfo->scrnx,binfo->scrny);

	
	
	

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

	buf_mouse=(uint32_t *)malloc(sizeof(uint32_t)*24*24);

	int x,y;

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

	sht_mouse=sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse,buf_mouse,24,24,DESKTOP_BACKCOLOR);
	
	int32_t mouse_x,mouse_y;
	mouse_x=binfo->scrnx/2;
	mouse_y=binfo->scrny/2;

	sheet_slide(sht_back,  0,  0);
	sheet_updown(sht_back,  0);
	sheet_slide(sht_mouse,  mouse_x,  mouse_y);
	sheet_updown(sht_mouse,  1);

	//putblock(binfo->vram,binfo->scrnx,24,24,mouse_x,mouse_y,mouse,24);
	uint8_t mouse_data;


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
				if (mouse_x > binfo->scrnx-24)
				{
					mouse_x = binfo->scrnx-24;
				}
				if (mouse_y > binfo->scrny-24)
				{
					mouse_y = binfo->scrny-24;
				}
				//putblock(binfo->vram,binfo->scrnx,24,24,mouse_x,mouse_y,mouse,24);
				sheet_slide(sht_mouse,  mouse_x,  mouse_y);
				sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
				if ((mdec.btn & 0x01) != 0) {
					s[1] = 'L';
				}
				if ((mdec.btn & 0x02) != 0) {
					s[3] = 'R';
				}
				if ((mdec.btn & 0x04) != 0) {
					s[2] = 'C';
				}
				boxfill(binfo->vram,binfo->scrnx,0,0,15*8,15,0xFFFFFF);
				putstr_ascii(binfo->vram,binfo->scrnx,0,0,0xFF0000,s);
			}
		}
		if(fifo_status(&decoded_key)>0)
		{
 			sprintf(s,"Keyboard Data:%c",fifo_get(&decoded_key));
			boxfill(binfo->vram,binfo->scrnx,0,16,15*8,16+15,0xFFFFFF);
			putstr_ascii(binfo->vram,binfo->scrnx,0,16,0xFF0000,s);
		}
		

	}
}

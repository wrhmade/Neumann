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
void init_desktop(uint32_t *buf,uint32_t xsize,uint32_t ysize)
{
    boxfill(buf,xsize,0,0,xsize-1,ysize-1,DESKTOP_BACKCOLOR);
	
    putstr_ascii(buf,xsize,xsize-24*8,ysize-77,ARGB(255,0,0,0),"Neumann Operating System");
    putstr_ascii(buf,xsize,xsize-19*8,ysize-61,ARGB(255,0,0,0),"Version 0.8[Beta 6]");
    putstr_ascii(buf,xsize,xsize-52*8,ysize-45,ARGB(255,0,0,0),"WARNING:THIS IS A BETA VERSION THAT MAY BE UNSTABLE.");
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
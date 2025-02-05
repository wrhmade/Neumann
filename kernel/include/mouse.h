/*
mouse.h
PS/2鼠标驱动程序头文件
Copyright W24 Studio 
*/

#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>
typedef struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
}mdec_t;

void init_ps2mouse(void);
int mouse_decode(mdec_t *mdec, uint8_t dat);

#endif
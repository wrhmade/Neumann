/*
graphic.h 
图形操作头文件 
Copyright W24 Studio 
*/

#ifndef GRAPHIC_H
#define GRAPHIC_H
#include <stdint.h>

#define ARGB(a,r,g,b) (a<<24) | (r<<16) | (g<<8) | b
void boxfill(uint32_t *vram,uint16_t xsize,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint32_t c);
void putfont(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *font);
void putstr_ascii(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, unsigned char *s);

#define DESKTOP_BACKCOLOR ARGB(255,57,109,165)
#endif
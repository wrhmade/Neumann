/*
graphic.h 
图形操作头文件 
Copyright W24 Studio 
*/

#ifndef GRAPHIC_H
#define GRAPHIC_H
#include <stdint.h>

#define ARGB(a,r,g,b) (a<<24) | (r<<16) | (g<<8) | b

uint32_t alphablend(uint32_t fc,uint32_t bc,uint8_t a);

void boxfill(uint32_t *vram,uint16_t xsize,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint32_t c);

void putfont(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *font);
void putstr_ascii(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *s);
void putstr_ascii_lmode(uint32_t *vram, uint16_t xsize, uint16_t x, uint16_t y, uint32_t c, char *s,int langmode);
void putfont_gb2312(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,char *font1,char *font2);
void putblock(uint32_t *vram, int vxsize, int pxsize,int pysize, int px0, int py0, uint32_t *buf, int bxsize);

void putfont_ttf(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,uint32_t bc,const unsigned char *ttf_buffer,int chr,uint32_t size);
void putstr_ttf(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,uint32_t bc,const unsigned char *ttf_buffer,char *s,uint32_t size);
void putstr_ttf_file(uint32_t *vram,uint16_t xsize,uint16_t x,uint16_t y,uint32_t c,uint32_t bc,char *ttf_filename,char *s,uint32_t size,int instead);

int hzk16_load(const char *filename);
int hzk16f_load(const char *filename);
#endif
/*
binfo.h
启动信息 
Copyright W24 Studio 
*/

#ifndef BINFO_H
#define BINFO_H
#include <stdint.h>
struct BOOTINFO
{
	uint16_t vmode,scrnx,scrny;
	uint32_t *vram;
} __attribute__((packed)); 
#endif

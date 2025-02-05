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
	uint32_t memtotal;
	char *hzk16,*hzk16f;
	uint32_t base_count;
} __attribute__((packed)); 
#endif

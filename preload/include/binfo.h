/*
binfo.h
启动信息 
Copyright W24 Studio 
*/

#ifndef BINFO_H
#define BINFO_H
#include <stdint.h>

#define DEVICE_HD 1
#define DEVICE_FD 0
#define DEVICE_RD 2

struct BOOTINFO
{
	uint16_t vmode,scrnx,scrny;
	uint32_t *vram;
	uint32_t memtotal;
	char *hzk16,*hzk16f;
	uint32_t base_count;
	uint32_t boot_device;
	uint32_t *vram_really;
	uint32_t *kernel_elf_base;
} __attribute__((packed));
#endif

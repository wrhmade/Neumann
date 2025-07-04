/*
mm.h
内存管理程序头文件
Copyright W24 Studio 
*/

#ifndef MM_H
#define MM_H
#include <stddef.h>

#define MEMMAN_ADDR 0x3c0000
#define MEMMAN_FREES		4090

#define MEMMAN_FREES 4090

typedef struct FREEINFO {
	uint32_t addr, size;
} freeinfo_t;


typedef struct MEMMAN {
	int frees;
	freeinfo_t free[MEMMAN_FREES];
} memman_t;

uint32_t memtest(uint32_t start, uint32_t end);
uint32_t free_space_total(void);

void *kmalloc(uint32_t size);
void kfree(void *p);
void *krealloc(void *buffer, int size);
uint32_t init_mem(void);
#endif
/*
mm.h
内存管理程序头文件
Copyright W24 Studio 
*/

#ifndef MM_H
#define MM_H
#include <stdint.h>

#define MEMMAN_ADDR 0x3c0000
#define MEMMAN_FREES		4090

typedef struct FREEINFO {	/* 偁偒忣曬 */
	uint32_t addr, size;
}freeinfo_t;
typedef struct MEMMAN {		/* 儊儌儕娗棟 */
	int32_t frees, maxfrees, lostsize, losts;
	freeinfo_t free[MEMMAN_FREES];
}memman_t;

uint32_t memtest(uint32_t start, uint32_t end);
uint32_t init_mem(void);
uint32_t free_space_total(void);

void *malloc(uint32_t size);
void free(void *p);
#endif
/*
dma.h
直接内存访问驱动程序头文件
Copyright W24 Studio 
*/

#ifndef DMA_H
#define DMA_H
#include <stddef.h>
void dma_send(byte channel, void *address, unsigned int size);
void dma_recv(byte channel, void *address, unsigned int size);
#endif
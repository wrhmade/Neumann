/*
fifo.c
队列管理程序头文件
Copyright W24 Studio 
*/

#ifndef FIFO_H
#define FIFO_H
#include <stdint.h>
typedef struct FIFO {
    uint32_t *buf;
    int p, q, size, free, flags;
} fifo_t;

#define FIFO_FLAGS_OVERRUN 1

void fifo_init(fifo_t *fifo, int size, uint32_t *buf);
int fifo_put(fifo_t *fifo, uint32_t data);
int fifo_get(fifo_t *fifo);
int fifo_status(fifo_t *fifo);
#endif
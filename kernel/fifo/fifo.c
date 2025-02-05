/*
fifo.c
队列管理程序
Copyright W24 Studio 
*/

#include <fifo.h>

void fifo_init(fifo_t *fifo, int size, uint32_t *buf)
{
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->p = 0;
    fifo->q = 0;
}

int fifo_put(fifo_t *fifo, uint32_t data)
{
    if (fifo->free == 0) {
        fifo->flags |= FIFO_FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->p] = data;
    fifo->p++;
    if (fifo->p == fifo->size) fifo->p = 0;
    fifo->free--;
    return 0;
}

int fifo_get(fifo_t *fifo)
{
    int data;
    if (fifo->free == fifo->size) return -1;
    data = fifo->buf[fifo->q];
    fifo->q++;
    if (fifo->q == fifo->size) fifo->q = 0;
    fifo->free++;
    return data;
}

int fifo_status(fifo_t *fifo)
{
    return fifo->size - fifo->free;
}
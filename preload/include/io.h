/*
io.h
I/O读写头文件
Copyright W24 Studio 
*/

#ifndef IO_H
#define IO_H
#include <stdint.h> 
uint32_t io_in8(int port);
uint32_t io_in16(int port);
uint32_t io_in32(int port);
void io_out8(int port,int data);
void io_out16(int port,int data);
void io_out32(int port,int data);
#endif

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
void io_out8(int port,uint8_t data);
void io_out16(int port,uint16_t data);
void io_out32(int port,uint32_t data);
void insw(uint16_t port, void *buf, unsigned long n);
void outsw(uint16_t port, const void *buf, unsigned long n);
void insl(uint32_t port, void *addr, int cnt);
void outsl(uint32_t port, const void *addr, int cnt);

void io_wrmsr(uint32_t msr, uint64_t value);
uint64_t io_rdmsr(uint32_t msr);

#endif

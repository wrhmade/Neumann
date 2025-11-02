/*
io.c
I/O端口操作
Copyright W24 Studio 
*/

#include <io.h>
#include <stdint.h>

uint32_t io_in8(int port)
{
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

uint32_t io_in16(int port)
{
    uint16_t value;
    asm volatile("inw %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

uint32_t io_in32(int port)
{
    uint32_t value;
    asm volatile("inl %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

void io_out8(int port,uint8_t data)
{
    asm volatile("outb %1, %0" ::"dN"(port), "a"(data));
}

void io_out16(int port,uint16_t data)
{
    asm volatile("outw %1, %0" ::"dN"(port), "a"(data));
}

void io_out32(int port,uint32_t data)
{
    asm volatile("outl %1, %0" ::"dN"(port), "a"(data));
}

/* 从I/O端口批量地读取数据到内存（16位） */
void insw(uint16_t port, void *buf, unsigned long n)
{
	__asm__ __volatile__("cld; rep; insw"
                 : "+D"(buf),
                 "+c"(n)
                 : "d"(port));
}

/* 从内存批量地写入数据到I/O端口（16位） */
void outsw(uint16_t port, const void *buf, unsigned long n)
{
	__asm__ __volatile__("cld; rep; outsw"
                 : "+S"(buf),
                 "+c"(n)
                 : "d"(port));
}

/* 从I/O端口批量地读取数据到内存（32位） */
void insl(uint32_t port, void *addr, int cnt)
{
	__asm__ __volatile__("cld;"
                 "repne; insl;"
                 : "=D" (addr), "=c" (cnt)
                 : "d" (port), "0" (addr), "1" (cnt)
                 : "memory", "cc");
}

/* 从内存批量地写入数据到I/O端口（32位） */
void outsl(uint32_t port, const void *addr, int cnt)
{
	__asm__ __volatile__("cld;"
                 "repne; outsl;"
                 : "=S" (addr), "=c" (cnt)
                 : "d" (port), "0" (addr), "1" (cnt)
                 : "memory", "cc");
}

uint64_t io_rdmsr(uint32_t msr)
{
    uint32_t eax,edx;
    asm volatile("rdmsr":"=a"(eax),"=d"(edx):"c"(msr));
    return ((uint64_t)edx<<32)|eax;
}

void io_wrmsr(uint32_t msr, uint64_t value)
{
    uint32_t eax=(uint32_t)value;
    uint32_t edx=value>>32;
    asm volatile("wrmsr"::"c"(msr),"a"(eax),"d"(edx));
}
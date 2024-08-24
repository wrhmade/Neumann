/*
timer.c
计时器驱动程序
Copyright W24 Studio 
*/

#include <int.h>
#include <timer.h>
#include <task.h>
#include <macro.h>
#include <io.h>
#include <stdio.h>
#include <binfo.h> 
#include <macro.h>
#include <graphic.h>

#define PIT_CTRL    0x0043
#define PIT_CNT0    0x0040

char s[60];
extern taskctl_t *taskctl;

static void timer_callback(registers_t *regs)
{
    task_switch();
    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    // sprintf(s,"Running:%d",taskctl->running);
    // boxfill(binfo->vram,binfo->scrnx,0,0,8*strlen(s)-1,15,0xFFFFFF);
    // putstr_ascii(binfo->vram,binfo->scrnx,0,0,0xFF0000,s);
    // sprintf(s,"Now:%d",taskctl->now);
    // boxfill(binfo->vram,binfo->scrnx,0,16,8*strlen(s)-1,31,0xFFFFFF);
    // putstr_ascii(binfo->vram,binfo->scrnx,0,16,0xFF0000,s);
}
void init_timer(uint32_t freq)
{
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);

    io_out8(PIC0_IMR, 0xf8);
    io_out8(PIC1_IMR, 0xef);

    register_interrupt_handler(IRQ0, &timer_callback); // 将时钟中断处理程序注册给IRQ框架


    uint32_t divisor = 1193180 / freq;

    io_out8(0x43, 0x36); // 指令位，写入频率

    uint8_t l = (uint8_t) (divisor & 0xFF); // 低8位
    uint8_t h = (uint8_t) ((divisor >> 8) & 0xFF); // 高8位

    io_out8(0x40, l);
    io_out8(0x40, h); // 分两次发出
}
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
#include <acpi.h>

#pragma GCC optimize("00") //硬件处理不开优化

#define PIT_CTRL    0x0043
#define PIT_CNT0    0x0040

char s[60];
extern taskctl_t *taskctl;
static int tick=0;

void timer_callback(registers_t *regs)
{
    task_switch();
    tick++;
    // sprintf(s,"Running:%d",taskctl->running);
    // boxfill(binfo->vram,binfo->scrnx,0,0,8*strlen(s)-1,15,0xFFFFFF);
    // putstr_ascii(binfo->vram,binfo->scrnx,0,0,0xFF0000,s);
    // sprintf(s,"Now:%d",taskctl->now);
    // boxfill(binfo->vram,binfo->scrnx,0,16,8*strlen(s)-1,31,0xFFFFFF);
    // putstr_ascii(binfo->vram,binfo->scrnx,0,16,0xFF0000,s);
}

int benchcpu()
{
    int base_count=0;
    uint32_t c=tick;
    while(tick - c < 100) {
            base_count++;
    }
    return base_count;
}

/* 使当前进程休眠指定的时间 */
void sleep(uint64_t timer)
{
	clock_sleep(timer);
}

/* 实现sleep函数的内部逻辑 */
void clock_sleep(uint64_t timer)
{
	uint64_t sleep = nano_time() + timer;
	while(1){
		
		if(nano_time() >= sleep) break;
	}
}
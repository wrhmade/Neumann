/*
timer.h
计时器驱动程序头文件
Copyright W24 Studio 
*/

#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
void init_timer(uint32_t freq);
void sleep(uint64_t timer);
void clock_sleep(uint64_t timer);
int benchcpu(void);
#endif
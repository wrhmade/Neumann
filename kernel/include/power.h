/*
power.h
电源管理头文件
Copyright W24 Studio 
*/

#ifndef POWER_H
#define POWER_H
void poweroff();
void reboot();

void scheduled_poweroff(int time);
#endif
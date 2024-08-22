/*
regctl.h
寄存器操作头文件
Copyright W24 Studio 
*/

#ifndef REGCTL_H
#define REGCTL_H
int load_eflags(void);
void store_eflags(int eflags);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
#endif
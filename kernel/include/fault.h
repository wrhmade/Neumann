/*
fault.h 
异常处理程序头文件
Copyright W24 Studio 
*/

#ifndef FAULT_H
#define FAULT_H
#include <int.h>
void fault_process(registers_t reg);
void print_stack_trace(void);
#endif
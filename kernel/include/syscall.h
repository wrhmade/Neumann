/*
syscall.h
系统调用头文件
Copyright W24 Studio 
*/
#ifndef SYSCALL_H
#define SYSCALL_H
typedef void *syscall_func_t;

void syscall_putchar(char c);

syscall_func_t syscall_table[] = {
    syscall_putchar
};
#endif
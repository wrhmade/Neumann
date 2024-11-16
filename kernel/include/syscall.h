/*
syscall.h
系统调用头文件
Copyright W24 Studio 
*/
#ifndef SYSCALL_H
#define SYSCALL_H
typedef void *syscall_func_t;

void syscall_putchar(char c);
void syscall_putstr(char *s);
void syscall_cls(char c);
void syscall_movcur(uint32_t pos);
int syscall_getkey();
int syscall_getconsxy();
void syscall_wait(int time);
void syscall_beep(int freq);

syscall_func_t syscall_table[] = {
    syscall_putchar,
    syscall_putstr,
    syscall_cls,
    syscall_movcur,
    syscall_getkey,
    syscall_getconsxy,
    syscall_wait,
    syscall_beep
};
#endif
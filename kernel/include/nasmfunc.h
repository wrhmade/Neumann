/*
nasmfunc.h
调用汇编函数头文件
Copyright W24 Studio 
*/

#ifndef MASMFUNC_H
#define MASMFUNC_H
void asm_hlt(void);
void asm_cli(void);
void asm_sti(void);
void asm_stihlt(void);
void start_app(int new_eip, int new_cs, int new_esp, int new_ss, int *esp0);
#endif

/*
lpt.h
打印机接口头文件
Copyright W24 Studio 
*/

#ifndef LPT_H
#define LPT_H

#define LPT_DATA(n) lpt_base[n]
#define LPT_STATUS(n) lpt_base[n]+1
#define LPT_CONTROL(n) lpt_base[n]+2

void lpt_put(unsigned char c,int i);
int lpt_read(int i);
void init_lpt();
#endif
/*
lpt.h
打印机接口头文件
Copyright W24 Studio 
*/

#ifndef LPT_H
#define LPT_H

#define LPT1_PORT_BASE 0x378    //LPT1接口基地址
#define LPT1_PORT_DATA LPT1_PORT_BASE   //LPT1数据接口
#define LPT1_PORT_STATUS LPT1_PORT_BASE+1   //LPT1状态接口
#define LPT1_PORT_CONTROL LPT1_PORT_BASE+2  //LPT1控制接口
void lpt_put(unsigned char c);
#endif
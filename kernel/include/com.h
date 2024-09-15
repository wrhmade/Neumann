/*
com.h
串口驱动程序头文件
Copyright W24 Studio 
*/

#ifndef COM_H
#define COM_H
#define SERIAL_PORT 0x3f8//COM串口地址
int init_com(void);
int serial_received(void);
char read_serial(void);
int is_transmit_empty(void);
void write_serial(char a);
void serial_putstr(char *s);
#endif
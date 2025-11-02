/*
com.h
串口驱动程序头文件
Copyright W24 Studio 
*/

#ifndef COM_H
#define COM_H
#define SERIAL_PORT 0x3f8//COM串口地址
int init_com(void);
int serial_received(int index);
char read_serial(int index);
int is_transmit_empty(int index);
void write_serial(char a);
void write_serial_index(char a,int index);
void serial_putstr(char *s);

void select_serial(int n);
void serial_putstr_index(char *s,int index);
#endif
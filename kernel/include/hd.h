/*
hd.h
IDE硬盘驱动程序头文件
Copyright W24 Studio 
*/

#ifndef HD_H
#define HD_H
void hd_read(int lba, int sec_cnt, void *buffer);
void hd_write(int lba, int sec_cnt, void *buffer);
int get_hd_sects();
#endif
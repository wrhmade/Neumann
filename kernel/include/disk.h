/*
disk.h
包括硬盘、软盘等的磁盘操作的头文件
Copyright W24 Studio 
*/

#ifndef DISK_H
#define DISK_H
void my_disk_write_device(int device,int lba,int cnt,void *buffer);
void my_disk_read_device(int device,int lba,int cnt,void *buffer);
void my_disk_write(int lba,int cnt,void *buffer);
void my_disk_read(int lba,int cnt,void *buffer);
#endif
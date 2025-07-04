/*
devfs.h
设备虚拟文件系统头文件
Copyright W24 Studio 
*/

#ifndef DEVFS_H
#define DEVFS_H

#include "vfs.h"

int devfs_mount(const char* src, vfs_node_t node);
void devfs_regist(void);
int dev_get_sector_size(char *path);
int dev_get_size(char *path);
int dev_get_type(char *path); // 1:HDD 2:CDROM
void print_devfs(void);
void devfs_sysinfo_init(void);

#endif // DEVFS_H

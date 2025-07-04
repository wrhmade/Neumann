/*
syscall.h
系统调用头文件
Copyright W24 Studio 
*/
#ifndef SYSCALL_H
#define SYSCALL_H
#include <dirent.h>
#include <fcntl.h>
#include <vfs.h>
#include <stdint.h>
typedef void *syscall_func_t;


int sys_getpid();

// file.h
int sys_open(char *filename, uint32_t flags);
int sys_write(int fd, const void *msg, int len);
int sys_read(int fd, void *buf, int count);
int sys_close(int fd);
int sys_lseek(int fd, int offset, uint8_t whence);
int sys_unlink(const char *filename);

int sys_mkdir(const char *path);
int sys_rmdir(const char *path);
DIR *sys_opendir(const char *name);
struct dirent *sys_readdir(DIR *dir);
void sys_rewinddir(DIR *dir);
void sys_closedir(DIR *dir);
int sys_fstat(int fd, struct stat *st);
char *sys_getcwd(char *buf, int size);
int sys_chdir(const char *path);

int get_fsize(int fd);
#endif
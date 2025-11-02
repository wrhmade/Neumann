/*
syscall.h
系统调用头文件
Copyright W24 Studio 
*/
#ifndef SYSCALL_H
#define SYSCALL_H
#include <dirent.h>
#include <vfs.h>
#include <stdint.h>
#include <task.h>
#include <cmos.h>
typedef struct NAPI_FILE_STATUS
{
    int size;
    int read;
    int write;
    int pos;
}napi_fstat_t;

void init_file_table();
void task_free_table(task_t *task);
int syscall_napi_getfilesize(int fd);
int syscall_napi_open_file(const char *filename,int flags);
int syscall_napi_read_file(int fd,void *buffer,int len);
int syscall_napi_file_status(int fd,napi_fstat_t *stat);
int syscall_npai_file_set_pos(int fd,int pos);
#endif
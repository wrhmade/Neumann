/*
syscall.h
系统调用头文件
Copyright W24 Studio 
*/
#ifndef SYSCALL_H
#define SYSCALL_H
typedef void *syscall_func_t;


int sys_getpid();
int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir);

// file.h
int sys_open(char *filename, uint32_t flags);
int sys_write(int fd, const void *msg, int len);
int sys_read(int fd, void *buf, int count);
int sys_close(int fd);
int sys_lseek(int fd, int offset, uint8_t whence);
int sys_unlink(const char *filename);

#endif
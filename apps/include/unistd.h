#ifndef UNISTD_H
#define UNISTD_H

#include <stdint.h>

int open(char *filename, uint32_t flags);
int write(int fd, const void *msg, int len);
int read(int fd, void *buf, int count);
int close(int fd);
int lseek(int fd, int offset, uint8_t whence);
int unlink(const char *filename);
int waitpid(int pid);
int exit(int ret);
void *sbrk(int incr);

int create_process(const char *app_name, const char *cmdline, const char *work_dir);

#endif
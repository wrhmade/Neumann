#ifndef NAPI_FILE_FILE_H
#define NAPI_FILE_FILE_H
#define NAPI_FILE_READ 1
#define NAPI_FILE_WRITE 2
#define NAPI_FILE_CREATE 4

#include "../time.h"
typedef struct NAPI_FILE_STATUS
{
    int size;
    int read;
    int write;
    int pos;
}napi_fstat_t;

int napi_open_file(const char *filename,int flags);
int napi_file_status(int fd,napi_fstat_t *stat);
int npai_file_set_pos(int fd,int pos);
int napi_read_file(int fd,void *buffer,int len);
#endif
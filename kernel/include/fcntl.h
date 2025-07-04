/*
fnctl.h
文件操作头文件
Copyright W24 Studio 
*/

#ifndef STAT_H
#define STAT_H

#include <stdint.h>
#include <time.h>

typedef enum FILE_TYPE {
    FT_USABLE,
    FT_REGULAR,
    FT_DIRECTORY,
    FT_UNKNOWN
} file_type_t;

typedef enum oflags {
    O_RDONLY,
    O_WRONLY,
    O_RDWR,
    O_CREAT = 4
} oflags_t;

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

struct stat {
    uint32_t st_size;
    file_type_t st_type;
    struct tm st_time;
};

int stat(const char *filename, struct stat *st);

#endif
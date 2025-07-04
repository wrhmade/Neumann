/*
dirent.h
目录的抽象表示 
Copyright W24 Studio 
*/
#ifndef DIRENT_H
#define DIRENT_H

#define MAX_FILE_NUM 512

struct dirent {
    char name[20]; // 给多了
    int size;
};

typedef struct {
    struct dirent dir_entries[MAX_FILE_NUM]; // 一个目录下最多这么多文件
    int entry_count; // 总共多少个目录项
    int pos;
} DIR;

#endif
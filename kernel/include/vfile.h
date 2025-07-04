/*
vfile.h
文件操作抽象层头文件
Copyright W24 Studio 
*/

#ifndef VFILE_H
#define VFILE_H
#include <console.h>
#include <fcntl.h>

#define MAX_FILE_NUM 512

typedef struct FILE_STRUCT {
    void *handle;
    void *buffer;
    int pos;
    int size;
    int open_cnt;
    file_type_t type;
    oflags_t flags;
} cfile_t;


typedef struct {
    int path_stack_top;
    char **path_stack;
} path_stack_t;

/* 将vfs_node_t结构体路径转为字符串 */
char *vfs_node_to_path(vfs_node_t node);

/* 切换工作目录 */
int file_cd(const char *path);

int file_ls(console_t *console,const char *path);
int file_mkdir(const char *path);

char *rel2abs(const char *path);
void path_parse(char *path, path_stack_t *path_stack);
void path_stack_deinit(path_stack_t *path_stack);
int install_to_global(vfs_node_t node);
int install_to_local(int global_fd);
#endif
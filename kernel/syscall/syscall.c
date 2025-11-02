/*
syscall.c
系统调用
Copyright W24 Studio 
*/
#include <stdint.h>
#include <syscall.h>
#include <task.h>
#include <console.h>
#include <timer.h>
#include <buzzer.h>
#include <fifo.h>
#include <stdio.h>
#include <string.h>
#include <mm.h>
#include <window.h>
#include <sheet.h>
#include <graphic.h>
#include <exec.h>
#include <cmos.h>
#include <dirent.h>
#include <stddef.h>
#include <vfile.h>
#include <list.h>
#include <cmos.h>
#include "malloc.c"
#define NAPI_FILE_READ 1
#define NAPI_FILE_WRITE 2
#define NAPI_FILE_CREATE 4

#define FILE_TABLE_NUM 255
typedef struct NAPI_FILE_TABLE
{
    vfs_node_t node;
    task_t *task;
    int read;
    int write;
    int flag;
    int pos;
}napi_file_table_t;
napi_file_table_t napi_file_table[FILE_TABLE_NUM];

int color_table[16] = {
    0xFF000000,0xFF0000FF,0xFF00FF00,0xFF00FFFF,
    0xFFFF0000,0xFF800080,0xFFFFFF00,0xFFAAAAAA,
    0xFF808080,0xFFADD8E6,0xFF98FB98,0xFFCCFFCC,
    0xFFFFA07A,0xFFDDA0DD,0xFFFFFFCC,0xFFFFFFFF
};

void init_file_table()
{
    for(int i=0;i<FILE_TABLE_NUM;i++)
    {
        napi_file_table[i].flag=0;
    }
}

void syscall_nmanager(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)//int 60h中断
{
    task_t *task=task_now();
    int ds_base = task->ds_base;
    console_t *console=task->window->console;
    int ret = 0;
    switch(eax)
    {
        case 0:task_exit(ebx);break;
        case 1:console_putchar(console,ebx);break;
        case 2:console_putstr(console,(char *)(ds_base+ebx));break;
        case 3:ret=(int)sys_sbrk(ebx);break;
        case 4:ret=console_getkey(console);break;
        case 5:char *p=console_input(console,ecx);strcpy((char *)(ds_base+ebx),p);kfree(p);break;
        case 6:console_cleanscreen(console);break;
        case 7:get_current_time((current_time_t *)(ds_base+ebx));break;
        case 8:ret=syscall_napi_open_file((const char *)(ds_base+ebx),ecx);break;
        case 9:ret=syscall_napi_file_status(ebx,(napi_fstat_t *)(ds_base+ecx));break;
        case 10:ret=syscall_npai_file_set_pos(ebx,ecx);break;
        case 11:ret=syscall_napi_read_file(ebx,(void *)(ds_base+ecx),edx);break;
    }
    int *save_reg = &eax + 1;
    save_reg[7] = ret;
}

static int alloc_ftable()
{
    for(int i=0;i<FILE_TABLE_NUM;i++)
    {
        if(napi_file_table[i].flag==0)
        {
            return i;
        }
    }
    return -1;
}

static void free_ftable(int index)
{
    napi_file_table[index].flag=0;
}

void task_free_table(task_t *task)
{
    for(int i=0;i<FILE_TABLE_NUM;i++)
    {
        if(napi_file_table[i].task==task)
        {
            free_ftable(i);
        }
    }
}

int syscall_napi_getfilesize(int fd)
{
    if(fd<0 || fd>=FILE_TABLE_NUM)
    {
        return -1;
    }
    return napi_file_table[fd].node->size;
}

int syscall_napi_open_file(const char *filename,int flags)
{
    int index=alloc_ftable();
    if(index==-1)//分配失败
    {
        return -1;
    }
    int isrel=0;
    char *p=(char *)filename;
    if(p[0]!='/')//相对路径
    {
        p=rel2abs(p);
        isrel=1;
    }
    vfs_node_t node=vfs_open(p);
    if(node==0)
    {
        if(flags&NAPI_FILE_CREATE)
        {
            if(vfs_mkfile(p)==-1)
            {
                goto err;
            }
            node=vfs_open(p);
            if(node==0)
            {
                goto err;
            }
        }
        else
        {
            goto err;
        }
    }
    napi_file_table[index].flag=1;
    napi_file_table[index].read=flags&NAPI_FILE_READ;
    napi_file_table[index].write=flags&NAPI_FILE_WRITE;
    napi_file_table[index].node=node;
    napi_file_table[index].task=task_now();
    napi_file_table[index].pos=0;
    if(isrel)
    {
        kfree(p);
    }
    return index;
err:
    if(isrel)
    {
        kfree(p);
    }
    return -1;
}

int syscall_napi_file_status(int fd,napi_fstat_t *stat)
{
    if(!stat)//stat结构体为空
    {
        return -1;
    }
    if(fd>=FILE_TABLE_NUM|| fd<0)//非法描述符
    {
        return -1;
    }
    napi_file_table_t file=napi_file_table[fd];
    if(file.flag==0)//空描述符
    {
        return -1;
    }
    vfs_node_t node=file.node;
    stat->size=node->size;
    stat->read=file.read;
    stat->write=file.write;
    stat->pos=file.pos;
    return 0;
}

int syscall_npai_file_set_pos(int fd,int pos)
{
    if(fd>=FILE_TABLE_NUM|| fd<0)//非法描述符
    {
        return -1;
    }
    napi_file_table_t file=napi_file_table[fd];
    if(file.flag==0)//空描述符
    {
        return -1;
    }
    int size=file.node->size;
    if(pos<0 || pos>=size)
    {
        return -1;
    }
    napi_file_table[fd].pos=pos;
    return pos;
}

int syscall_napi_read_file(int fd,void *buffer,int len)
{
    if(fd>=FILE_TABLE_NUM|| fd<0)//非法描述符
    {
        return -1;
    }
    napi_file_table_t file=napi_file_table[fd];
    if(file.flag==0 && !file.read)//空描述符或是不可读文件
    {
        return -1;
    }
    int ret=vfs_read(file.node,buffer,file.pos,len);
    return ret;
}
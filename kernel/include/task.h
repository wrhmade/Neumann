/*
task.h
多任务管理程序头文件
Copyright W24 Studio 
*/

#ifndef TASK_H
#define TASK_H
#include <stdint.h>
#include <fifo.h>
#define MAX_FILE_OPEN_PER_TASK 32
typedef struct WINDOW window_t;
 
typedef struct TSS32 {
    uint32_t backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldtr, iomap;
} tss32_t;


typedef struct exit_retval {
    int pid, val;
} exit_retval_t;
 
typedef struct TASK {
    uint32_t sel;
    int32_t flags;
    tss32_t tss;
    window_t *window;
    fifo_t fifo;
    uint32_t *fifobuf;
    exit_retval_t my_retval;
    int fd_table[MAX_FILE_OPEN_PER_TASK];
    int langmode;//0为ASCII英文，1为GB2312中文
    unsigned char langbyte;
} task_t;
 
#define MAX_TASKS 1000
#define TASK_GDT0 3

 
typedef struct TASKCTL {
    int running, now;
    task_t *tasks[MAX_TASKS];
    task_t tasks0[MAX_TASKS];
} taskctl_t;

 
task_t *task_init();
task_t *task_alloc();
void task_run(task_t *task);
void task_switch();
task_t *task_now();
int task_pid(task_t *task);
void task_exit(int value);
int task_wait(int pid);
task_t *create_kernel_task(void *entry);
#endif
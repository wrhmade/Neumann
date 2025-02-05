/*
task.c
多任务管理程序
Copyright W24 Studio 
*/

#include <stdint.h> 
#include <task.h>
#include <gdtidt.h>
#include <int.h>
#include <io.h>
#include <stddef.h>
#include <regctl.h>
#include <macro.h>
#include <binfo.h>
#include <fifo.h>

extern void farjmp(int,int);
 
taskctl_t *taskctl;
 
task_t *task_init()
{
    task_t *task;
    taskctl = (taskctl_t *) malloc(sizeof(taskctl_t));
    for (int i = 0; i < MAX_TASKS; i++) {
        taskctl->tasks0[i].flags = 0;
        taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
        taskctl->tasks0[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
        gdt_set_gate(TASK_GDT0 + i, (int) &taskctl->tasks0[i].tss, 103, 0x89); // 硬性规定，0x89 代表 TSS，103 是因为 TSS 共 26 个 uint32_t 组成，总计 104 字节，因规程减1变为103
        gdt_set_gate(TASK_GDT0 + MAX_TASKS + i, (int) &taskctl->tasks0[i].ldt, 15, 0x82); // 0x82 代表 LDT，两个 GDT 表项共计 16 字节
        
    }
    task = task_alloc();
    task->flags = 2;
    taskctl->running = 1;
    taskctl->now = 0;
    taskctl->tasks[0] = task;
    load_tr(task->sel); // 向CPU报告当前task->sel对应的任务为正在运行的任务

    return task;
}


task_t *task_alloc()
{
    task_t *task;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (taskctl->tasks0[i].flags == 0) {
            task = &taskctl->tasks0[i];
            task->flags = 1;
            task->tss.eflags = 0x00000202;
            task->tss.eax = task->tss.ecx = task->tss.edx = task->tss.ebx = 0;
            task->tss.ebp = task->tss.esi = task->tss.edi = 0;
            task->tss.es = task->tss.ds = task->tss.fs = task->tss.gs = 0;
            task->tss.iomap = 0x40000000;
            task->my_retval.pid = -1;      // 这里是新增的部分
            task->my_retval.val = -114514; // 这里是新增的部分
            task->window=NULL;
            task->fifobuf=(uint32_t *)malloc(sizeof(uint32_t)*128);
            fifo_init(&task->fifo,128,task->fifobuf);
            task->fd_table[0] = 0; // 标准输入，占位
            task->fd_table[1] = 1; // 标准输出，占位
            task->fd_table[2] = 2; // 标准错误，占位
            for (int i = 3; i < MAX_FILE_OPEN_PER_TASK; i++) {
                task->fd_table[i] = -1; // 其余文件均可用
            }
            task->langmode=0;
            task->langbyte=0;
            return task;
        }
    }
    return NULL;
}

void task_switch()
{
    if (taskctl->running >= 2) { // 显然，至少得有两个任务才能切换
        taskctl->now++; // 下一个任务
        if (taskctl->now == taskctl->running) { // 到结尾了
            taskctl->now = 0; // 转换为第一个
        }
        farjmp(0, taskctl->tasks[taskctl->now]->sel); // 跳入任务对应的 TSS
    }
}


task_t *task_now()
{
    return taskctl->tasks[taskctl->now];
}


void task_run(task_t *task)
{
    task->flags = 2;
    taskctl->tasks[taskctl->running] = task;
    taskctl->running++;
}

void task_remove(task_t *task)
{
    bool need_switch = false; // 是否要进行切换？
    int i;
    if (task->flags == 2) { // 此任务正在运行，如果不运行那就根本不在tasks里，什么都不用干
        if (task == task_now()) { // 是当前任务
            need_switch = true; // 待会还得润
        }
        for (i = 0; i < taskctl->running; i++) {
            if (taskctl->tasks[i] == task) break; // 在tasks中找到当前任务
        }
        taskctl->running--; // 运行任务数量减1
        if (i < taskctl->now) taskctl->now--; // 如果now在这个任务的后面，那now也要前移一个（因为这个任务要删了，后面的要填上来，会整体前移一个）
        for (; i < taskctl->running; i++) {
            taskctl->tasks[i] = taskctl->tasks[i + 1]; // 整体前移，不必多说
        }
        if (need_switch) { // 需要切换
            if (taskctl->now >= taskctl->running) {
                taskctl->now = 0; // now超限，重置为0
            }
            farjmp(0, task_now()->sel); // 跳入到现在的当前任务中
        }
    }
}


void task_exit(int value)
{
    task_t *cur = task_now(); // 当前任务
    cur->my_retval.pid = task_pid(cur); // pid变为当前任务的pid
    cur->my_retval.val = value; // val为此时的值
    task_remove(cur); // 删除当前任务
    cur->flags = 4; // 返回值还没人收，暂时还不能释放这个块为可用（0）
}



int task_wait(int pid)
{
    task_t *task = &taskctl->tasks0[pid]; // 找出对应的task
    while (task->my_retval.pid == -1); // 若没有返回值就一直等着
    task->flags = 0; // 释放为可用
    return task->my_retval.val; // 拿到返回值
}  

int task_pid(task_t *task)
{
    return task->sel / 8 - TASK_GDT0;
}



task_t *create_kernel_task(void *entry)
{
    task_t *new_task;
    new_task = task_alloc();
    new_task->tss.esp = (uint32_t) malloc(64 * 1024) + 64 * 1024 - 4;
    new_task->tss.eip = (int) entry;
    new_task->tss.es = new_task->tss.ss = new_task->tss.ds = new_task->tss.fs = new_task->tss.gs = 2 * 8;
    new_task->tss.cs = 1 * 8;
    return new_task;
}

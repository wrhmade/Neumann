/*
fpu.c
浮点处理器驱动程序
Copyright W24 Studio 
*/

#include <fpu.h>
#include <cpu.h>
#include <stddef.h>
#include <task.h>
#include <regctl.h>
#include <gdtidt.h>
#include <mm.h>

#pragma GCC optimize("00") //硬件处理不开优化

task_t *last_fpu_task = NULL;

bool check_fpu()
{
    cpu_version_t ver;
    cpu_version(&ver);
    if (!ver.FPU)//如果FPU不存在
        return false;
    uint32_t testword = 0x55AA;
    uint32_t ret;
    asm volatile(
        "movl %%cr0, %%edx\n" // 获取 cr0 寄存器
        "andl %%ecx, %%edx\n" // 清除 EM TS 保证 FPU 可用
        "movl %%edx, %%cr0\n" // 设置 cr0 寄存器

        "fninit\n"    // 初始化 FPU
        "fnstsw %1\n" // 保存状态字到 testword

        "movl %1, %%eax\n" // 将状态字保存到 eax
        : "=a"(ret)        // 将 eax 写入 ret;
        : "m"(testword), "c"(-1 - CR0_EM - CR0_TS));
    return ret == 0; // 如果状态被改为 0 则 FPU 可用
}

// 激活 fpu
void fpu_enable(task_t *task)
{
    // LOGK("fpu enable...\n");

    store_cr0(load_cr0() & ~(CR0_EM | CR0_TS));
    // 如果使用的任务没有变化，则无需恢复浮点环境
    if (last_fpu_task == task)
        return;

    // 如果存在使用过浮点处理单元的进程，则保存浮点环境
    if (last_fpu_task && last_fpu_task->flags & TASK_FPU_ENABLED)
    {
        //assert(last_fpu_task->fpu);
        asm volatile("fnsave (%%eax) \n" ::"a"(last_fpu_task->fpu));
        last_fpu_task->flags &= ~TASK_FPU_ENABLED;
    }

    last_fpu_task = task;

    // 如果 fpu 不为空，则恢复浮点环境
    if (task->fpu)
    {
        asm volatile("frstor (%%eax) \n" ::"a"(task->fpu));
    }
    else
    {
        // 否则，初始化浮点环境
        asm volatile(
            "fnclex \n"
            "fninit \n");

        //LOGK("FPU create state for task 0x%p\n", task);
        task->fpu = (fpu_t *)kmalloc(sizeof(fpu_t));
        task->flags |= (TASK_FPU_ENABLED | TASK_FPU_USED);
    }
}

// 禁用 fpu
void fpu_disable(task_t *task)
{
    store_cr0(load_cr0() | (CR0_EM | CR0_TS));
}

void fpu_handler(int vector)
{
    // LOGK("fpu handler...\n");
    //assert(vector == INTR_NM);
    task_t *task = task_now();
    //assert(task->uid);

    fpu_enable(task);
}

bool init_fpu()
{
    bool exists=check_fpu();
    last_fpu_task=NULL;
    if(exists)
    {
        uint32_t cr0=load_cr0();
        cr0 |= CR0_EM | CR0_TS | CR0_NE;
        store_cr0(cr0);
        return true;
    }
    else
    {
        return false;
    }
}
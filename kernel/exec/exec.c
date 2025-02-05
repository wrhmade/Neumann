/*
exec.c
运行应用程序
Copyright W24 Studio 
*/
#include <task.h>
#include <exec.h>
#include <mm.h>
#include <nasmfunc.h>
#include <stdio.h>
#include <fat16.h>
#include <stddef.h>

int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir, window_t *window)
{
    task_t *old_task=task_now();
    int fd = sys_open((char *) app_name, O_RDONLY);
    if (fd == -1) return -1;
    sys_close(fd);
    task_t *new_task = create_kernel_task(app_entry);
    window_settask(window, new_task);
    new_task->tss.esp -= 12;
    new_task->langmode=old_task->langmode;
    *((int *) (new_task->tss.esp + 4)) = (int) app_name;
    *((int *) (new_task->tss.esp + 8)) = (int) cmdline;
    *((int *) (new_task->tss.esp + 12)) = (int) work_dir;
    task_run(new_task);
    return task_pid(new_task);
}

void app_entry(const char *app_name, const char *cmdline, const char *work_dir)
{
    char s[50] = {0};
    int fd = sys_open((char *) app_name, O_RDONLY);
    int size = sys_lseek(fd, -1, SEEK_END) + 1;
    sys_lseek(fd, 0, SEEK_SET);
    char *buf = (char *) malloc(size + 5);
    sys_read(fd, buf, size + 5);
    task_now()->ds_base = (int) buf; // 这里是新增的
    exec_ldt_set_gate(0, (int) buf, size-1, 0x409a | 0x60);
    exec_ldt_set_gate(1, (int) buf, size-1, 0x4092 | 0x60);
    start_app(0, 0 * 8 + 4, 0, 1 * 8 + 4, &(task_now()->tss.esp0));
    while (1);
}

void exec_ldt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint16_t ar)
{
    task_t *task = task_now();
    if (limit > 0xfffff) { // 段上限超过1MB
        ar |= 0x8000; // ar的第15位（将被当作limit_high中的G位）设为1
        limit /= 0x1000; // 段上限缩小为原来的1/4096，G位表示段上限为实际的4KB
    }
    // base部分没有其他的奇怪东西混杂，很好说
    task->ldt[num].base_low = base & 0xFFFF; // 低16位
    task->ldt[num].base_mid = (base >> 16) & 0xFF; // 中间8位
    task->ldt[num].base_high = (base >> 24) & 0xFF; // 高8位
    // limit部分混了一坨ar进来，略微复杂
    task->ldt[num].limit_low = limit & 0xFFFF; // 低16位
    task->ldt[num].limit_high = ((limit >> 16) & 0x0F) | ((ar >> 8) & 0xF0); // 现在的limit最多为0xfffff，所以最高位只剩4位作为低4位，高4位自然被ar的高12位挤占

    task->ldt[num].access_right = ar & 0xFF; // ar部分只能存低4位了
}

int try_to_run_external(char *name, int *exist,char *cmd_line, window_t *window)
{
    int ret = sys_create_process(name, cmd_line, "/", window); // 尝试执行应用程序
    *exist = false; // 文件不存在
    
    if (ret == -1) { // 哇真的不存在
        char new_name[100] = {0}; // 由于还没有实现malloc，所以只能这么搞，反正文件最长就是MAX_CMD_LEN这么长
        strcpy(new_name, name); // 复制文件名
        int len = strlen(name); // 文件名结束位置
        new_name[len] = '.'; // 给后
        new_name[len + 1] = 'b'; // 缀加
        new_name[len + 2] = 'i'; // 上个
        new_name[len + 3] = 'n'; // .bin
        new_name[len + 4] = '\0'; // 结束符
        ret = sys_create_process(new_name, cmd_line, "/", window); // 第二次尝试执行应用程序
        if (ret == -1) return -1; // 文件还是不存在，那只能不存在了
    }
    *exist = true; // 错怪你了，文件存在
    ret = task_wait(ret); // 等待直到这个pid的进程返回并拿到结果
    return ret; // 把返回值返回回去
}
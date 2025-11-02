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
#include <stddef.h>
#include <ELF.h>
#include <console.h>
#include <sheet.h>
#include <gdtidt.h>
#include <syscall.h>
#include <vfile.h>

extern shtctl_t *global_shtctl;

void app_kill(sheet_t *sht)//强行结束应用程序
{
    task_t *task=sht->window->console->running_app;
    task->my_retval.val=-1;
    task_remove(task);
    sht->window->console->running_app=NULL;
    
}

static void expand_user_segment(int increment)
{
    task_t *task = task_now();
    if (!task->is_user) return; // 内核都打满4GB了还需要扩容？
    gdt_entry_t *segment = &task->ldt[1];
    // 接下来把base和limit的石块拼出来
    uint32_t base = segment->base_low | (segment->base_mid << 16) | (segment->base_high << 24); // 其实可以不用拼直接用ds_base 但还是拼一下吧当练习
    uint32_t size = segment->limit_low | ((segment->limit_high & 0x0F) << 16);
    if (segment->limit_high & 0x80) size *= 0x1000;
    size++;
    // 分配新的内存
    void *new_base = (void *) kmalloc(size + increment + 5);
    if (increment > 0) return; // expand是扩容你缩水是几个意思
    memcpy(new_base, (void *) base, size); // 原来的内容全复制进去
    // 用户进程的base必然由malloc分配，故用free释放之
    kfree((void *) base);
    // 那么接下来就是把new_base设置成新的段了
    exec_ldt_set_gate(1, (int) new_base, size + increment - 1, 0x4092 | 0x60); // 反正只有数据段允许扩容我也就设置成数据段算了
    task->ds_base = (int) new_base; // 既然ds_base变了task里的应该同步更新
}

int sys_create_process(const char *app_name, const char *cmdline, const char *work_dir, window_t *window)
{
    task_t *old_task=task_now();
    char *p=(char *)app_name;
    int isrel=0;
    if(p[0]!='/')
    {
        p=rel2abs(p);
        isrel=1;
    }
    vfs_node_t node=vfs_open(p);
    if(isrel)
    {
        kfree(p);
    }
    if(node==0)
    {
        return -1;
    }
    task_t *new_task = create_kernel_task(app_entry);
    window_settask(window, new_task);
    new_task->tss.esp -= 12;
    new_task->langmode=old_task->langmode;
    kfree(new_task->work_dir);
    new_task->work_dir=kmalloc(strlen(old_task->work_dir)+5);
    strcpy(new_task->work_dir,old_task->work_dir);
    if(task_now()->window->console)
    {
        task_now()->window->console->running_app=new_task;
    }
    name_task(new_task,app_name);
    *((int *) (new_task->tss.esp + 4)) = (int) app_name;
    *((int *) (new_task->tss.esp + 8)) = (int) cmdline;
    *((int *) (new_task->tss.esp + 12)) = (int) work_dir;
    task_run(new_task);
    int pid=task_pid(new_task);
    int i;
    sheet_t *sht;
    for(i=0;i<MAX_SHEETS;i++)
    {
        sht = &(global_shtctl->sheets0[i]);
        if ((sht->flags & 0x11) == 0x11)
        {
            if(sht->window!=NULL)
            {
                if(sht->window->task==new_task && sht->window->task!=old_task)
                {
                    close_window(sht->window);
                }
            }
            else
            {
                if(sht->task==new_task && sht->task!=old_task)
                {
                    sheet_free(sht);
                }
            }
        }
    }
    task_now()->window->console->running_app=NULL;
    return pid;
}


void *sys_sbrk(int incr)
{
    task_t *task = task_now();
    if (task->is_user) { // 是应用程序
        if (task->brk_start + incr > task->brk_end) { // 如果超出已有缓冲区
            expand_user_segment(incr + 32 * 1024); // 再多扩展32KB
            task->brk_end += incr + 32 * 1024; // 由于扩展了32KB，同步将brk_end移到现在的数据段结尾
        }
        void *ret = task->brk_start; // 旧的program break
        task->brk_start += incr; // 直接添加就完事了
        return ret; // 返回之
    }
    return NULL; // 非用户不允许使用sbrk
}

void app_entry(const char *app_name, const char *cmdline, const char *work_dir)
{
    char *p=(char *)app_name;
    int isrel=0;
    if(p[0]!='/')
    {
        p=rel2abs(p);
        isrel=1;
    }
    vfs_node_t node=vfs_open(p);
    if(node==0)
    {
        if(isrel)
        {
            kfree(p);
        }
        task_exit(-1);
    }
    char *buf = (char *) kmalloc(node->size + 5);
    vfs_read(node,buf,0,node->size);
    if(isrel)
    {
        kfree(p);
    }
    uint32_t first,last;
    char *code;
    int entry = load_elf((Elf32_Ehdr *) buf, &code, &first, &last); // buf是文件读进来的那个缓冲区，code是存实际代码的
    if (entry == -1) // 解析失败，直接exit(-1)
    {
        console_putstr(task_now()->window->console,"App Format Error.\n");
        kfree(buf);
        task_exit(-1);
    }
    char *ds = (char *) kmalloc(last - first + 4 * 1024 * 1024 + 5); // 新分配一个数据段，为原来大小+4MB+5
    memcpy(ds, code, last - first); // 把代码复制过来，也就包含了必须要用的数据
    task_now()->is_user = true;
    task_now()->brk_start = (void *) last - first + 4 * 1024 * 1024;
    task_now()->brk_end = (void *) last - first + 5 * 1024 * 1024 - 1;
    int new_esp = last - first + 4 * 1024 * 1024 - 4;
    int prev_brk = (int)sys_sbrk(strlen(cmdline) + 5); // 分配cmdline这么长的内存，反正也输入不了1MB长的命令
    strcpy((char *) (ds + prev_brk), cmdline); // sys_sbrk使用相对地址，要转换成以ds为基址的绝对地址需要加上ds
    *((int *) (ds + new_esp)) = (int) prev_brk; // 把prev_brk的地址写进栈里，这个位置可以被_start访问
    new_esp -= 4; // esp后移一个dword
    task_now()->ds_base = (int) ds; // 设置ds基址
    exec_ldt_set_gate(0, (int) code, last - first - 1, 0x409a | 0x60);
    exec_ldt_set_gate(1, (int) ds, last - first + 4 * 1024 * 1024 + 1 * 1024 * 1024 - 1, 0x4092 | 0x60);
    start_app(entry, 0 * 8 + 4, new_esp, 1 * 8 + 4, (int *)(&(task_now()->tss.esp0)));
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
    char cmdline_back[CMDLINE_MAXLEN];
    strcpy(cmdline_back, cmd_line); // 这里是新增加的
    int ret = sys_create_process(name, cmdline_back, "/", window); // 尝试执行应用程序
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
        ret = sys_create_process(new_name, cmdline_back, "/", window); // 第二次尝试执行应用程序
        if (ret == -1) return -1; // 文件还是不存在，那只能不存在了
    }
    *exist = true; // 错怪你了，文件存在
    ret = task_wait(ret); // 等待直到这个pid的进程返回并拿到结果
    return ret; // 把返回值返回回去
}
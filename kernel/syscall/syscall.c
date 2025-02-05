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
#include <fat16.h>
#include <stdio.h>

extern file_t file_table[];

void syscall_manager(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
    int ds_base = task_now()->ds_base;
    int ret = 0;
    switch (eax) {
        case 0:
            ret = sys_getpid();
            break;
        case 1:
            ret = sys_write(ebx, (char *) ecx + ds_base, edx);
            break;
        case 2:
            ret = sys_read(ebx, (char *) ecx + ds_base, edx);
            break;
        case 3: // 从这里开始
            ret = sys_open((char *) ebx + ds_base, ecx);
            break;
        case 4:
            ret = sys_close(ebx);
            break;
        case 5:
            ret = sys_lseek(ebx, ecx, edx);
            break;
        case 6:
            ret = sys_unlink((char *) ebx + ds_base);
            break;
        case 7:
            ret = sys_create_process((const char *) ebx + ds_base, (const char *) ecx + ds_base, (const char *) edx + ds_base);
            break;
        case 8:
            ret = task_wait(ebx);
            break;
        case 9:
            task_exit(ebx);
            break; // 到这里结束
    }
    int *save_reg = &eax + 1;
    save_reg[7] = ret;
}

int sys_getpid()
{
    return task_pid(task_now());
}

extern fifo_t decoded_key; // 加在开头

// 省略中间的 syscall_manager、sys_getpid 和 sys_write

int sys_write(int fd, const void *msg, int len)
{
    if (fd <= 0) return -1; // 是无效fd，返回
    if (fd == 1 || fd == 2) { // 往标准输出或标准错误中输出
        console_putstr(task_now()->window->console,(char *)msg); // 直接用monitor_put逐字符输出
        return len; // 一切正常
    }
    task_t *task = task_now(); // 获取当前任务
    int global_fd = task->fd_table[fd]; // 获取文件表中索引
    file_t *cfile = &file_table[global_fd]; // 获取文件表中的文件指针
    if (cfile->flags == O_RDONLY) return -1; // 只读，不可写，返回
    for (int i = 0; i < len; i++) { // 对于每一个字节
        if (cfile->pos >= cfile->size) { // 如果超出了原本的范围
            cfile->size++; // 大小+1
            void *new_buffer = realloc(cfile->buffer, cfile->size); // 使用krealloc扩容，增长缓冲区大小
            if (new_buffer) cfile->buffer = new_buffer; // 如果缓冲区分配成功，那么这里就是新的缓冲区
        }
        char *buf = (char *) cfile->buffer; // 文件的缓冲区，相当于文件的当前内容了
        char *content = (char *) msg; // 要写入的内容
        buf[cfile->pos] = content[i]; // 向读写指针处写入当前内容
        cfile->pos++; // 文件指针后移
    }
    int status = fat16_write_file(cfile->handle, cfile->buffer, cfile->size); // 写入完毕，立刻更新到硬盘
    if (status == -1) return status; // 写入失败，返回
    return len; // 否则，返回实际写入的长度len
}

int sys_read(int fd, void *buf, int count)
{
    int ret = -1;
    if (fd < 0 || fd == 1 || fd == 2) return ret; // 从标准输入/标准错误中读或是fd非法都是不允许的
    if (fd == 0) { // 如果是标准输入
        char *buffer = (char *) buf; // 先转成char *
        uint32_t bytes_read = 0; // 读了多少个
        while (bytes_read < count) { // 没达到count个
            while (fifo_status(&decoded_key) == 0); // 只要没有新的键我就不读进来
            *buffer = fifo_get(&decoded_key); // 获取新的键
            bytes_read++;
            buffer++; // buffer指向下一个
        }
        ret = (bytes_read == 0 ? -1 : (int) bytes_read); // 如果啥也没读着就-1，否则就正常返回就行了
        return ret;
    }
    task_t *task = task_now(); // 获取当前任务
    int global_fd = task->fd_table[fd]; // 获取fd对应的文件表索引
    file_t *cfile = &file_table[global_fd]; // 获取文件表中对应文件
    if (cfile->flags == O_WRONLY) return -1; // 只写，不可读，返回-1
    ret = 0; // 记录到底读了多少个字节
    for (int i = 0; i < count; i++) {
        if (cfile->pos >= cfile->size) break; // 如果已经到达末尾，返回
        char *filebuf = (char *) cfile->buffer; // 文件缓冲区
        char *retbuf = (char *) buf; // 接收缓冲区
        retbuf[i] = filebuf[cfile->pos]; // 逐字节拷贝内容
        cfile->pos++; // 读写指针后移
        ret++; // 读取字节数+1
    }
    return ret; // 返回读取字节数
}


int sys_open(char *filename, uint32_t flags)
{
    fileinfo_t finfo; // 准备接收打开的文件
    if (flags & O_CREAT) { // flags中含有O_CREAT，则需要创建文件
        int status = fat16_create_file(&finfo, filename); // 调用创建文件的函数
        if (status == -1) return status; // 创建失败则直接不管
    } else {
        int status = fat16_open_file(&finfo, filename); // 调用打开文件的函数
        if (status == -1) return status; // 打开失败则直接不管
    }
    int global_fd = install_to_global(finfo); // 先安装到全局文件表
    file_table[global_fd].open_cnt++; // open个数+1，没什么用
    file_table[global_fd].size = finfo.size; // 设置文件大小
    file_table[global_fd].flags = flags | (~O_CREAT); // flags中剔除O_CREAT
    file_table[global_fd].buffer = malloc(finfo.size + 5); // 分配一个缓冲区
    file_table[global_fd].pos = 0;
    if (finfo.size) { // 如果有内容
        int status = fat16_read_file(&finfo, file_table[global_fd].buffer); // 则直接读到缓冲区里来
        if (status == -1) { // 如果读不进缓冲区，那就只好这样了
            free(file_table[global_fd].handle); // 释放占有的资源
            free(file_table[global_fd].buffer);
            return status;
        }
    }
    return install_to_local(global_fd); // 最后安装到任务里
}

int sys_lseek(int fd, int offset, uint8_t whence)
{
    if (fd < 3) return -1; // 不是被打开的文件，返回
    if (whence < 0 || whence > 2) return -1; // whence只能为012，分别对应SET、CUR、END，返回
    task_t *task = task_now(); // 获取当前任务
    file_t *cfile = &file_table[task->fd_table[fd]]; // 获取fd对应的文件
    fileinfo_t *fhandle = (fileinfo_t *) cfile->handle; // 文件实际上对应的fileinfo
    int size = fhandle->size; // 获取大小，总归是有用的
    int new_pos = 0; // 新的文件位置
    switch (whence) {
        case SEEK_SET: // SEEK_SET就是纯设置
            new_pos = offset; // 直接设置
            break;
        case SEEK_CUR: // 从当前位置算起移动offset位置
            new_pos = cfile->pos + offset; // 用当前pos加上offset
            break;
        case SEEK_END: // 从结束位置算起移动offset位置
            new_pos = size + offset; // 用大小加上offset
            break;
    }
    if (new_pos < 0 || new_pos > size - 1) return -1; // 如果新的位置超出文件，返回-1
    cfile->pos = new_pos; // 设置新位置
    return new_pos; // 返回新位置
}

int sys_unlink(const char *filename)
{
    return fat16_delete_file((char *) filename); // 直接套皮，不多说
}

int sys_close(int fd)
{
    int ret = -1; // 返回值
    if (fd > 2) { // 的确是被打开的文件
        task_t *task = task_now(); // 获取当前任务
        uint32_t global_fd = task->fd_table[fd]; // 获取对应文件表索引
        task->fd_table[fd] = -1; // 释放文件描述符
        file_t *cfile = &file_table[global_fd]; // 获取对应文件
        free(cfile->buffer); // 释放缓冲区
        free(cfile->handle); // install_to_global中使用kmalloc分配fileinfo指针
        cfile->type = FT_USABLE; // 设置type为可用
        return 0; // 关闭完成
    }
    return ret; // 否则返回-1
}

void syscall_nmanager(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)//int 60h中断
{
    int ds_base = task_now()->ds_base;
    console_t *console=task_now()->window->console;
    int ret = 0;
    switch(eax)
    {
        case 0:
            /*
                退出程序
                eax = 0
                ebx = 返回值
            */
            task_exit(ebx);
            break;
        case 1:
            /*
                控制台输出文本
                eax = 1
                ebx = 文本
            */
            if(console!=NULL)
            {
                console_putstr(console,(char *)(ds_base+ebx));
            }
            else
            {
                ret=-1;
            }
            break;
        case 2:
            /*
                控制台输出彩色文本
                eax = 2
                ebx = 文本
                ecx = 文本颜色
            */
            if(console!=NULL)
            {
                console_putstr_color(console,(char *)(ds_base+ebx),ecx);
            }
            else
            {
                ret=-1;
            }
            break;
        case 3:
            /*
                控制台读取字符
                eax = 3
                输出：eax = 读取的字符
            */
            if(console!=NULL)
            {
                ret=console_getkey(console);
            }
            break;
    }
    int *save_reg = &eax + 1;
    save_reg[7] = ret;
}
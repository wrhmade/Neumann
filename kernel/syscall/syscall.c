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


#include "malloc.c"

extern cfile_t file_table[];

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
            ret = sys_create_process((const char *) ebx + ds_base, (const char *) ecx + ds_base, (const char *) edx + ds_base,task_now()->window);
            break;
        case 8:
            ret = task_wait(ebx);
            break;
        case 9:
            task_exit(ebx);
            break; // 到这里结束
         case 10:
            ret = (int) sys_sbrk(ebx);
            break;
        case 11:
            ret = (int) sys_opendir((char *) ((const char *) ebx + ds_base));
            break;
        case 12:
            ret = (int) sys_readdir((DIR *) ((const char *) ebx + ds_base));
            break;
        case 13:
            sys_rewinddir((DIR *) ((const char *) ebx + ds_base));
            break;
        case 14:
            sys_closedir((DIR *) ebx);
            break;
        case 15:
            ret = sys_mkdir((const char *) ebx + ds_base);
            break;
        case 16:
            ret = sys_rmdir((const char *) ebx + ds_base);
            break;
        case 17:
            ret = sys_fstat(ebx, (struct stat *) ((char *) ecx + ds_base));
            break;
        case 18:
            ret = sys_chdir((const char *) ebx + ds_base);
            break;
        case 19:
            ret = (int) sys_getcwd((char *) ebx + ds_base, ecx);
            break;
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
    cfile_t *cfile = &file_table[global_fd]; // 获取文件表中的文件指针
    if (cfile->flags == O_RDONLY) return -1; // 只读，不可写，返回
    for (int i = 0; i < len; i++) { // 对于每一个字节
        if (cfile->pos >= cfile->size) { // 如果超出了原本的范围
            cfile->size++; // 大小+1
            void *new_buffer = krealloc(cfile->buffer, cfile->size); // 使用krealloc扩容，增长缓冲区大小
            if (new_buffer) cfile->buffer = new_buffer; // 如果缓冲区分配成功，那么这里就是新的缓冲区
        }
        char *buf = (char *) cfile->buffer; // 文件的缓冲区，相当于文件的当前内容了
        char *content = (char *) msg; // 要写入的内容
        buf[cfile->pos] = content[i]; // 向读写指针处写入当前内容
        cfile->pos++; // 文件指针后移
    }
    int status = vfs_write(cfile->handle,cfile->buffer,0,cfile->size); // 写入完毕，立刻更新到硬盘
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
    cfile_t *cfile = &file_table[global_fd]; // 获取文件表中对应文件
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
    int is_relative = 0;
    // 先处理相对路径
    if (filename[0] != '/') {
        is_relative = 1;
        filename = rel2abs(filename);
    }
	vfs_node_t node;
	if(flags & O_CREAT)
	{
		node=vfs_mkfile(filename);
	}
	else
	{
		node=vfs_open(filename);
	}
	
	if(node==0)
	{
		if (is_relative) kfree(filename);
		return -1;
	}
	int global_fd=install_to_global(node);
    file_table[global_fd].open_cnt++; // open个数+1，没什么用
    file_table[global_fd].size = node->size; // 设置文件大小
    file_table[global_fd].flags = flags | (~O_CREAT); // flags中剔除O_CREAT
    file_table[global_fd].buffer = kmalloc(node->size + 5); // 分配一个缓冲区
    if (node->size) { // 如果有内容
        int status = vfs_read(node,file_table[global_fd].buffer,0,node->size); // 则直接读到缓冲区里来
        if (status == -1) { // 如果读不进缓冲区，那就只好这样了
            kfree(file_table[global_fd].handle); // 释放占有的资源
            kfree(file_table[global_fd].buffer);
            return status;
        }
    }
    if (is_relative) kfree(filename);
    return install_to_local(global_fd); // 最后安装到任务里s	
}

int sys_close(int fd)
{
    int ret = -1; // 返回值
    if (fd > 2) { // 的确是被打开的文件
        task_t *task = task_now(); // 获取当前任务
        uint32_t global_fd = task->fd_table[fd]; // 获取对应文件表索引
        task->fd_table[fd] = -1; // 释放文件描述符
        cfile_t *cfile = &file_table[global_fd]; // 获取对应文件
        kfree(cfile->buffer); // 释放缓冲区
        kfree(cfile->handle); // install_to_global中使用kmalloc分配fileinfo指针
        cfile->type = FT_USABLE; // 设置type为可用
        return 0; // 关闭完成
    }
    return ret; // 否则返回-1
}

int sys_lseek(int fd, int offset, uint8_t whence)
{
    if (fd < 3) return -1; // 不是被打开的文件，返回
    if (whence < 0 || whence > 2) return -1; // whence只能为012，分别对应SET、CUR、END，返回
    task_t *task = task_now(); // 获取当前任务
    cfile_t *cfile = &file_table[task->fd_table[fd]]; // 获取fd对应的文件
    vfs_node_t fhandle = (vfs_node_t) cfile->handle; // 文件实际上对应的fileinfo
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
	int is_relative = 0;
    // 先处理相对路径
    if (filename[0] != '/') {
        is_relative = 1;
        filename = rel2abs(filename);
    }
    vfs_node_t node=vfs_open(filename);
    if (node == 0 || (node->type==file_dir)) {
        if (is_relative) kfree((char *) filename);
        return -1;
    }
	vfs_free(node);// 直接套皮，不多说    
	if (is_relative) kfree(filename);
    return 0;
}

int sys_mkdir(const char *path)
{
	int is_relative = 0;
    // 先处理相对路径
    if (path[0] != '/') {
        is_relative = 1;
        path = rel2abs(path);
    }

	int status=vfs_mkdir(path);	   
	if(status==-1)return -1;

	if (is_relative) kfree(path);
    return 0;
}

int sys_rmdir(const char *path)
{
	int is_relative = 0;
    // 先处理相对路径
    if (path[0] != '/') {
        is_relative = 1;
        path = rel2abs(path);
    }

	vfs_node_t node=vfs_open(path);
	if(node->type!=file_dir)
	{
		if (is_relative) kfree(path);
    	return -1;
	}

	if(list_length(node->child)!=0)//必须为空目录
	{
		if (is_relative) kfree(path);
    	return -1;
	}

	vfs_free(node);

	if (is_relative) kfree(path);
    return 0;
}

DIR *sys_opendir(const char *name)
{
 	int is_relative = 0;
    // 先处理相对路径
    if (name[0] != '/') {
        is_relative = 1;
        name = rel2abs(name);
    }

	vfs_node_t node=vfs_open(name);
	if(node->type!=file_dir)
	{
		if (is_relative) kfree(name);
    	return -1;
	}

	DIR *ret = (DIR *) malloc(sizeof(DIR));
    // 现在是在r0的段 所以ret要加上ds_base才能正确更新到r3的ret里
    ret = (DIR *) ((char *) ret + (task_now()->ds_base));
	memset(ret, 0, sizeof(DIR));
    ret->pos = 0;

	list_t list=node->child;
	int ret_entry_index;
	for(int i=0;i<list_length(list);i++)
	{
		vfs_node_t entry=list_nth(list,i)->data;
		ret->dir_entries[ret_entry_index].size = entry->size;
        int ret_name_index = 0;
        // 处理文件名
        for (int j = 0; j < 8; j++) {
            char alpha = entry->name[j];
            if (alpha == ' ') break;
            if (alpha >= 'A' && alpha <= 'Z') alpha += 0x20;
            ret->dir_entries[ret_entry_index].name[ret_name_index++] = alpha;
        }
        ret_entry_index++;
	}



	ret->entry_count = ret_entry_index;
    // 把ret减回去
    ret = (DIR *) ((char *) ret - (task_now()->ds_base));
	if (is_relative) kfree(name);
    return ret;
}

struct dirent *sys_readdir(DIR *dir)
{
    // C语言笑传之查查边
    if (dir->pos >= dir->entry_count) return NULL;
    struct dirent *ret = &dir->dir_entries[dir->pos++];
    ret = (struct dirent *) ((char *) ret - (task_now()->ds_base));
    return ret;
}

void sys_rewinddir(DIR *dir)
{
    dir->pos = 0;
}

void sys_closedir(DIR *dir)
{
    free((char *) dir - (task_now()->ds_base));
}

int sys_fstat(int fd, struct stat *st)
{
    // 从fd获取fileinfo_t
    int global_fd = task_now()->fd_table[fd];
    vfs_node_t node = (vfs_node_t) file_table[global_fd].handle;
    // 向stat结构体填充信息
    st->st_size = node->size;
    if (node->type==file_dir) st->st_type = FT_DIRECTORY;
    else st->st_type = FT_REGULAR;
    // st->st_time.tm_year = ((node->createtime & 0xfe00) >> 9) + 1980;
    // st->st_time.tm_month = (finfo->date & 0x01e0) >> 5;
    // st->st_time.tm_mday = finfo->date & 0x001f;
    // st->st_time.tm_hour = (finfo->time & 0xf800) >> 11;
    // st->st_time.tm_min = (finfo->time & 0x07e0) >> 5;
    // st->st_time.tm_sec = finfo->time & 0x001f;
    return 0;
}

char *sys_getcwd(char *buf, int size)
{
    task_t *task = task_now();
    buf -= task->ds_base;
    char *res = buf;
    if (size && strlen(task->work_dir) >= size) return NULL; // 装不下
    if (!size && buf) return NULL; // 大小为0又不malloc，你要干什么！
    if (!buf) {
        if (size) res = malloc(size);
        else res = malloc(strlen(task->work_dir) + 5);
    }
    res += task->ds_base;
    strcpy(res, task->work_dir);
    res -= task->ds_base;
    return res;
}

int sys_chdir(const char *path)
{
    task_t *task = task_now();
    int is_relative = 0;
    if (path[0] != '/') {
        is_relative = 0;
        path = rel2abs(path);
    }
    vfs_node_t node=vfs_open(path);
    if (node == 0 || !(node->type==file_dir)) {
        if (is_relative) kfree(path);
        return -1;
    }
    kfree(task->work_dir);
    task->work_dir = kmalloc(strlen(path) + 5);
    strcpy(task->work_dir, path);
    if (is_relative) kfree(path);
    return 0;
}

int color_table[16] = {
    0x000000,0x0000FF,0x00FF00,0x00FFFF,
    0xFF0000,0x800080,0xFFFF00,0xAAAAAA,
    0x808080,0xADD8E6,0x98FB98,0xCCFFCC,
    0xFFA07A,0xDDA0DD,0xFFFFCC,0xFFFFFF
};

void syscall_nmanager(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)//int 60h中断
{
    int ds_base = task_now()->ds_base;
    task_t *task=task_now();
    console_t *console=task_now()->window->console;
    int ret = 0;
    window_t *window;
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
                console_putstr_color(console,(char *)(ds_base+ebx),color_table[ecx]);
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
            else
            {
                ret=-1;
            }
            break;
        case 4:
            /*
                控制台读取字符串
                eax = 4
                ebx = 缓冲区
                ecx = 读取字符数
            */
            if(console!=NULL)
            {
                char *buf=(char *)(ds_base+ebx),*t;
                int len=ecx;
                t=console_input(console,len);
                strcpy(buf,t);
                kfree(t);
                ret=strlen(buf);
            }
            else
            {
                ret=-1;
            }
            break;
         case 5:
            /*
                控制台输出一个字符
                eax = 5
                ebx = 字符
            */
            if(console!=NULL)
            {
                console_putchar(console,ebx);
                ret=ebx;
            }
            else
            {
                ret=-1;
            }
            break;
        case 6:
            /*
                控制台输出一个彩色字符
                eax = 6
                ebx = 字符
                ecx = 字符颜色
            */
            if(console!=NULL)
            {
                console_putchar(console,color_table[ecx]);
                ret=ebx;
            }
            else
            {
                ret=-1;
            }
            break;
        case 7:
            /*
                创建一个带关闭按钮的窗口
                eax = 7
                ebx = 标题
                ecx = 宽
                edx = 高
                输出：eax = 窗口地址
            */
            window=create_window((char *)(ds_base+ebx),ecx,edx,-1,1);
            window->sheet->flags |= 0x10;
            window_settask(window,task);
            ret=(uint32_t)window;
            break;
        case 8:
            /*
                创建一个不带关闭按钮的窗口
                eax = 8
                ebx = 标题
                ecx = 宽
                edx = 高
                输出：eax = 窗口地址
            */
            window=create_window((char *)(ds_base+ebx),ecx,edx,-1,0);
            window->sheet->flags |= 0x10;
            window_settask(window,task);
            ret=(uint32_t)window;
            break;
        case 9:
            /*
                关闭窗口
                eax = 9
                ebx = 窗口地址
            */
           close_window((window_t *)ebx);
           break;
        case 10:
            /*
                在窗口上打印文字
                eax = 10
                ebx = 横坐标
                ecx = 纵坐标
                edx = 颜色
                edi = 窗口地址
                esi = 字符串        
            */
           window=(window_t *)edi;
           putstr_ascii(window->sheet->buf,window->xsize,ebx,ecx,edx,(char *)(ds_base+esi));
           sheet_refresh(window->sheet,0,0,window->xsize-1,window->ysize-1);
           break;
        case 11:
            /*
                获取命令行
                eax = 11
                ebx = 缓冲区   
            */
           ret=0;
           if(console!=NULL)
           {
                strcpy((char *)(ds_base+ebx),console->cmdline);
           }
           else
           {
                ret=-1;
           }
           break;
    }
    int *save_reg = &eax + 1;
    save_reg[7] = ret;
}

int get_fsize(int fd)
{
	task_t *task=task_now();
	int global_fd=task->fd_table[fd];
	return file_table[global_fd].size;	
}
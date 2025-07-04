/*
vfile.c
文件操作抽象层
Copyright W24 Studio 
*/


#include <vfs.h>
#include <string.h>
#include <vfile.h>
#include <task.h>
#include <mm.h>
#include <console.h>
#include <list.h>

cfile_t file_table[MAX_FILE_NUM];

/* 将vfs_node_t结构体路径转为字符串 */
char *vfs_node_to_path(vfs_node_t node)
{
	if (node == 0) {
		return 0;
	}
	if (node->parent == 0) {
		char* path = strdup("/");
		return path;
	} else {
		char* parent_path = vfs_node_to_path(node->parent);
		if (parent_path == 0) {
			return 0;
		}
		char* path = (char *)kmalloc(strlen(parent_path) + strlen(node->name) + 2);
		if (path == 0) {
			kfree(parent_path);
			return 0;
		}
		strcpy(path, parent_path);
		if (strcmp(parent_path, "/") != 0) {
			strcat(path, "/");
		}
		strcat(path, node->name);
		kfree(parent_path);
		return path;
	}
}

/* 切换工作目录 */
int file_cd(const char *path)
{
	int is_relative=0;
	task_t *task=task_now();
	char *p=(char *)path;
	if(path[0]!='/')
	{
		p=rel2abs(p);
		is_relative=1;
	}
	vfs_node_t node=vfs_open(p);
	if(node==0)
	{
		if(is_relative)
		{
			kfree(p);
		}
		return -1;
	}
	char *new=vfs_node_to_path(node);
	kfree(task->work_dir);
	task->work_dir=kmalloc(strlen(new)+5);
	strcpy(task->work_dir,new);
	kfree(new);
	if(is_relative)
	{
		kfree(p);
	}
	return 0;
}

/* 列出制定目录下的文件 */
int file_ls(console_t *console,const char *path)
{
	vfs_node_t p = vfs_open(path);
	if (p) {
		list_foreach(p->child, i) {
			char s[50];
			vfs_node_t c = (vfs_node_t)i->data;
			if(c->type==file_dir)
			{
				sprintf(s,"<%s>",c->name);
			}
			else
			{
				sprintf(s,"[%s]",c->name);
			}
			console_putstr(console,s);
			console_putchar(console,' ');
		}
		console_putchar(console,'\n');
		return 0;
	} else {
		return -1;
	}
}

int file_mkdir(const char *path)
{
	int is_relative=0;
	task_t *task=task_now();
	char *p=(char *)path;
	if(path[0]!='/')
	{
		p=rel2abs(p);
		is_relative=1;
	}
	if(vfs_mkdir(p)==-1)
	{
		if(is_relative)
		{
			kfree(p);
		}
		return -1;
	}
	if(is_relative)
	{
		kfree(p);
	}
	return 0;
}



char *rel2abs(const char *path)
{
    char *abspath = (char *) kmalloc(strlen(task_now()->work_dir) + strlen(path) + 6);
    strcpy(abspath, task_now()->work_dir);
	strcat(abspath, "/");
    strcat(abspath, path);
    path_stack_t path_stack; path_stack.path_stack_top = 0;
    path_parse(abspath, &path_stack);
    memset(abspath, 0, strlen(abspath));
    abspath[0] = '/';
    for (int i = 0; i < path_stack.path_stack_top; i++) {
        strcat(abspath, path_stack.path_stack[i]);
		if(i != path_stack.path_stack_top - 1)
		{
			strcat(abspath, "/");
		}
    }
    // 至此相对路径已转换为绝对路径存储至abspath
    path_stack_deinit(&path_stack);
    return abspath;
}


void path_parse(char *path, path_stack_t *path_stack)
{
    path_stack->path_stack_top = 0;
    path_stack->path_stack = (char **) kmalloc(strlen(path) * sizeof(char *)); // 初始化栈
    if (path[0] != '/') { // 第一个不是/，对后续处理会有影响
        char *new_path = (char *) kmalloc(strlen(path) + 5); // 从今天起你就是新的path了
        strcpy(new_path, "/"); // 先复制一个/
        strcat(new_path, path); // 再把后续的路径拼接上
        path = new_path; // 夺舍
    }
    char *level_start = path; // 当前路径层级的起始
    char *level_end = level_start + 1; // 当前路径层级的结尾
    while (*level_end) { // 直到还没到结尾
        while (*level_end != '/' && *level_end) {
            level_end++; // 遍历直到抵达`/`
        }
        int level_len = level_end - level_start; // 这一级路径的长度（前/计后/不计）
        if (level_len == 1) { // 如果就只有后面的一个/
            level_start = level_end; // start变为现在的end
            level_end = level_start + 1; // end变为现在的start+1
            continue; // 下一层
        }
        path_stack->path_stack[path_stack->path_stack_top] = kmalloc(level_len); // 初始化这一层路径栈
        char *p = level_start + 1; // 跳过本层路径一开始的/
        strncpy(path_stack->path_stack[path_stack->path_stack_top], p, level_len - 1); // 将本层路径拷入路径栈，只拷level_len - 1（去掉一开头的/）的长度
        if (!strcmp(path_stack->path_stack[path_stack->path_stack_top], "..")) { // 如果是..
            kfree(path_stack->path_stack[path_stack->path_stack_top]); // 首先释放新的这一层
            path_stack->path_stack_top--; // 然后弹栈
            kfree(path_stack->path_stack[path_stack->path_stack_top]); // 然后旧的那一层也就可以释放了
            if (path_stack->path_stack_top < 0) path_stack->path_stack_top = 0; // 如果都弹到结尾了，那你还真是nb，避免溢出
        } else if (!strcmp(path_stack->path_stack[path_stack->path_stack_top], ".")) {
            kfree(path_stack->path_stack[path_stack->path_stack_top]); // 如果是.，那就相当于白压了，释放即可
        } else path_stack->path_stack_top++; // 否则就正常入栈
        if (!*level_end) break; // 如果已经到达结尾，直接break，不要指望一开始的while
        level_start = level_end; // start变为现在的end
        level_end = level_start + 1; // end变为start+1
    }
}

// 回收path_stack
void path_stack_deinit(path_stack_t *path_stack)
{
    for (int i = 0; i < path_stack->path_stack_top; i++) kfree(path_stack->path_stack[i]);
    kfree(path_stack->path_stack);
}


int install_to_global(vfs_node_t node)
{
    int i = MAX_FILE_NUM;
    for (i = 0; i < MAX_FILE_NUM; i++) {
        if (file_table[i].type == FT_USABLE) break; // 当前文件空闲，则占用
    }
    if (i == MAX_FILE_NUM) return -1; // 没有文件空闲，则退出
    vfs_node_t safer_node = (vfs_node_t) kmalloc(sizeof(vfs_node_t)); // 分配一个finfo指针，准备挂到handle上
    if (safer_node==0) return -1;
    *safer_node = *node; // 装入
    file_table[i].handle = safer_node; // 这就是其内部的handle
    file_table[i].type = FT_REGULAR; // 类型为正常文件
    file_table[i].pos = 0; // 由于刚刚注册，pos设为0
    return i; // 返回其在文件表内的索引
}

int install_to_local(int global_fd)
{
    task_t *task = task_now(); // 获取当前任务
    int i;
    for (i = 3; i < MAX_FILE_OPEN_PER_TASK; i++) { // fd 0 1 2分别代表标准输入 标准输出 标准错误，所以从3开始找起
        if (task->fd_table[i] == -1) break; // 这里还空着，直接用
    }
    if (i == MAX_FILE_OPEN_PER_TASK) return -1; // 到达任务可打开的文件上限，返回-1
    task->fd_table[i] = global_fd; // 将文件表索引安装到任务的文件描述符表
    return i; // 返回索引，这就是对应的文件描述符了
}
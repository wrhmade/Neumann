/*
fat.c
FAT文件系统
Copyright W24 Studio 
*/

//***此代码来自Uinxed-kernel

#include <fat.h>
#include "fatfs/ff.h"
#include <vfs.h>
#include <mm.h>
#include <string.h>
#include <console.h>
#include <task.h>
#include <cmos.h>
#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

#define assert(condition) \
	do { \
		if (!(condition)) { \
			char s[300]; \
			sprintf(s,"Assertion failed, %s line:%d\n", __FILE__, __LINE__); \
			krnlcons_putstr(s); \
		} \
	} while (0)

#define error(msg) char s[300]; \
			sprintf(s,"[FatErr]: %s\n",msg); \
			krnlcons_putstr(s); \

static FATFS volume[10];
static vfs_node_t drive_number_mapping[10] = {0};
static int fatfs_id = 0;
typedef struct file {
	char *path;
	void *handle;
} *file_t;

static int alloc_number(void)
{
	for (int i = 0; i < 10; i++)
		if (drive_number_mapping[i] == 0) return i;
	error("No available drive number");
	return -1;
}

vfs_node_t fatfs_get_node_by_number(int number)
{
	if (number < 0 || number >= 10) return 0;
	return drive_number_mapping[number];
}

int fatfs_mkdir(void *parent, const char* name, vfs_node_t node)
{
	file_t p = parent;
	char  *new_path = kmalloc(strlen(p->path) + strlen(name) + 1 + 1);
	sprintf(new_path, "%s/%s", p->path, name);
	FRESULT res = f_mkdir(new_path);
	kfree(new_path);
	if (res != FR_OK) { return -1; }
	return 0;
}

int fatfs_mkfile(void *parent, const char* name, vfs_node_t node)
{
	file_t p = parent;
	char  *new_path = kmalloc(strlen(p->path) + strlen(name) + 1 + 1);
	sprintf(new_path, "%s/%s", p->path, name);
	FIL fp;
	FRESULT res = f_open(&fp, new_path, FA_CREATE_NEW);
	f_close(&fp);
	kfree(new_path);
	if (res != FR_OK) { return -1; }
	return 0;
}

int fatfs_readfile(file_t file, void *addr, size_t offset, size_t size)
{
	if (file == 0 || addr == 0) return -1;
	FRESULT res;
	res = f_lseek(file->handle, offset);
	if (res != FR_OK) return -1;
	uint32_t n;
	res = f_read(file->handle, addr, size, &n);
	if (res != FR_OK) return -1;
	return 0;
}

int fatfs_writefile(file_t file, const void *addr, size_t offset, size_t size)
{
	if (file == 0 || addr == 0) return -1;
	FRESULT res;
	res = f_lseek(file->handle, offset);
	if (res != FR_OK) return -1;
	uint32_t n;
	res = f_write(file->handle, addr, size, &n);
	if (res != FR_OK) return -1;
	return 0;
}

void fatfs_open(void *parent, const char* name, vfs_node_t node)
{
	file_t p = parent;
	char *new_path = kmalloc(strlen(p->path) + strlen(name) + 1 + 1);
	file_t new = kmalloc(sizeof(struct file));
	sprintf(new_path, "%s/%s", p->path, name);
	void *fp = 0;
	FILINFO fno;
	FRESULT res = f_stat(new_path, &fno);
	assert(res == FR_OK);
  vfs_node_t t;
	if (fno.fattrib & AM_DIR) {
		node->type = file_dir;
		fp = kmalloc(sizeof(DIR));
		p->handle=fp;
		res = f_opendir(fp, new_path);
		assert(res == FR_OK);
		for (;;) {
			/* 读取目录下的内容，再读会自动读下一个文件 */
			res = f_readdir(fp, &fno);
			/* 为空时表示所有项目读取完毕，跳出 */
			if (res != FR_OK || fno.fname[0] == 0) break;
			t=vfs_child_append(node, fno.fname, 0);
			t->size=fno.fsize;
      		if(fno.fattrib & AM_DIR)
      		{
        		t->type=file_dir;
      		} 
			file_t child=t->handle;
			child=kmalloc(sizeof(struct file));
			child->path=kmalloc(strlen(new_path)+strlen(fno.fname)+10);
			assert(child->path!=NULL);
			sprintf(child->path,"%s/%s",new_path,fno.fname);
		}
	} else {
		node->type = file_block;
		fp = kmalloc(sizeof(FIL));
		res = f_open(fp, new_path, FA_READ | FA_WRITE);
		node->size = f_size((FIL *)fp);
		assert(res == FR_OK);
	}
	assert(fp != 0);
	new->handle = fp;
	new->path = new_path;
	node->handle = new;
}

void fatfs_close(file_t handle)
{
	FILINFO fno;
	FRESULT res = f_stat(handle->path, &fno);
	assert(res == FR_OK);
	if (fno.fattrib & AM_DIR) {
		res = f_closedir(handle->handle);
	} else {
		res = f_close(handle->handle);
	}
	kfree(handle->path);
	kfree(handle->handle);
	kfree(handle);
	assert(res == FR_OK);
}

int fatfs_mount(const char* src, vfs_node_t node)
{
	if (!src) return -1;
	int drive = alloc_number();
	assert(drive != -1);
	drive_number_mapping[drive] = vfs_open(src);
	assert(drive_number_mapping[drive] != 0);
	char *path = kmalloc(3);
	bzero(path, 0);
	sprintf(path, "%d:", drive);
	FRESULT r = f_mount(&volume[drive], path, 1);
	assert(r==FR_OK);
	if (r != FR_OK) {
		vfs_close(drive_number_mapping[drive]);
		drive_number_mapping[drive] = 0;
		kfree(path);
		return -1;
	}
	file_t f = kmalloc(sizeof(struct file));
	f->path = path;
	DIR *h = kmalloc(sizeof(DIR));
	f_opendir(h, path);
	f->handle = h;
	node->fsid = fatfs_id;
	FILINFO fno;
	FRESULT res;
  vfs_node_t t;
	for (int i=1;;i++) {
		/* 读取目录下的内容，再读会自动读下一个文件 */
		res = f_readdir(h, &fno);
		/* 为空时表示所有项目读取完毕，跳出 */
		if (res != FR_OK || fno.fname[0] == 0) break;
		t=vfs_child_append(node, fno.fname, 0);
		t->size=fno.fsize;
		file_t child=t->handle;
		child=kmalloc(sizeof(struct file));
		child->path=kmalloc(strlen(path)+strlen(fno.fname)+10);
		assert(child->path!=NULL);
		sprintf(child->path,"%s/%s",path,fno.fname);
    if(fno.fattrib & AM_DIR)
    {
      t->type=file_dir;
    }

	}
	node->handle = f;
	return 0;
}

void fatfs_umount(void *root)
{
	file_t f = root;
	int number = f->path[0] - '0';
	drive_number_mapping[number] = 0;
	f_closedir(f->handle);
	f_unmount(f->path);
	kfree(f->path);
	kfree(f->handle);
	kfree(f);
}

int fatfs_stat(void *handle, vfs_node_t node)
{
	file_t  f = handle;
	FILINFO fno;
	FRESULT res = f_stat(f->path, &fno);
	if (res != FR_OK) return -1;
	if (fno.fattrib & AM_DIR) {
		node->type = file_dir;
	} else {
		node->type = file_block;
		node->size = fno.fsize;
	}
	return 0;
}

static struct vfs_callback callbacks = {
	.mount = fatfs_mount,
	.umount = fatfs_umount,
	.open = fatfs_open,
	.close = (vfs_close_t)fatfs_close,
	.read = (vfs_read_t)fatfs_readfile,
	.write = (vfs_write_t)fatfs_writefile,
	.mkdir = fatfs_mkdir,
	.mkfile = fatfs_mkfile,
	.stat = fatfs_stat,
};

void fatfs_regist(void)
{
	fatfs_id = vfs_regist("fatfs", &callbacks);
	//print_succ("Fat File System initialize.\n");
}


DWORD get_fattime()
{
	current_time_t ctime;
	get_current_time(&ctime);
	    return (DWORD)(ctime.year - 80) << 25 |
           (DWORD)(ctime.month + 1) << 21 |
           (DWORD)ctime.day << 16 |
           (DWORD)ctime.hour << 11 |
           (DWORD)ctime.min << 5 |
           (DWORD)ctime.sec >> 1;
}
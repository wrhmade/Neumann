/*
mm.c
内存管理程序
Copyright W24 Studio 
*/

#include <mm.h>
#include <regctl.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <com.h>

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

uint32_t memtest(uint32_t start, uint32_t end)
{
	char flg486 = 0;
	unsigned long int eflg, cr0, i;

	/* 386偐丄486埲崀側偺偐偺妋擣 */
	eflg = load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	store_eflags(eflg);
	eflg = load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 386偱偼AC=1偵偟偰傕帺摦偱0偵栠偭偰偟傑偆 */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* 僉儍僢僔儏嬛巭 */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* 僉儍僢僔儏嫋壜 */
		store_cr0(cr0);
	}

	return i;
}

uint32_t free_space_total(void)
/*计算可用空间*/
{
	memman_t *man=(memman_t *)MEMMAN_ADDR;
	uint32_t i, t = 0;
    char s[30];
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

static void memman_init(memman_t *man)
{
    man->frees = 0;
}

static uint32_t memman_alloc(memman_t *man, uint32_t size)
{
    uint32_t i, a;
    char s[40];
    for (i = 0; man->frees; i++) {
        if (man->free[i].size >= size) { // 找到了足够的内存
            a = man->free[i].addr;
            man->free[i].addr += size; // addr后移，因为原来的addr被使用了
            man->free[i].size -= size; // size也要减掉
            if (man->free[i].size == 0) { // 这一条size被分配完了
                man->frees--; // 减一条frees
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i + 1]; // 各free前移
                }
            }
            sprintf(s,"Memory Allocated,address:0x%p\n",a);
            serial_putstr(s);
            return a; // 返回
        }
    }
    return 0; // 无可用空间
}

static int memman_free(memman_t *man, uint32_t addr, uint32_t size)
{
    int i, j;
    char s[40];
    for (i = 0; i < man->frees; i++) {
        // 各free按addr升序排列
        if (man->free[i].addr > addr) break; // 找到位置了！
        // 现在的这个位置是第一个在addr之后的位置，有man->free[i - 1].addr < addr < man->free[i].addr
    }
    if (i > 0) {
        if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
            // 可以和前面的可用部分合并
            man->free[i - 1].size += size; // 并入
            if (i < man->frees) {
                if (addr + size == man->free[i].addr) {
                    // 可以与后面的可用部分合并
                    man->free[i - 1].size += man->free[i].size;
                    // man->free[i]删除不用
                    man->frees--; // frees减1
                    for (; i < man->frees; i++) {
                        man->free[i] = man->free[i + 1]; // 前移
                    }
                }
            }
            sprintf(s,"Memory Freed,address:0x%p\n",addr);
            serial_putstr(s);
            return 0; // free完毕
        }
    }
    // 不能与前面的合并
    if (i < man->frees) {
        if (addr + size == man->free[i].addr) {
            // 可以与后面的可用部分合并
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0; // 成功合并
        }
    }
    // 两边都合并不了
    if (man->frees < MEMMAN_FREES) {
        // free[i]之后的后移，腾出空间
        for (j = man->frees; j > i; j--) man->free[j] = man->free[j - 1];
        man->frees++;
        man->free[i].addr = addr;
        man->free[i].size = size; // 更新当前地址和大小
        return 0; // 成功合并
    }
    // 无free可用且无法合并
    return -1; // 失败
}



void *kmalloc(uint32_t size)
{
    uint32_t addr;
    memman_t *memman = (memman_t *) MEMMAN_ADDR;
    addr = memman_alloc(memman, size + 16); // 多分配16字节
    memset((void *) addr, 0, size + 16);
    char *p = (char *) addr;
    if (p) {
        *((int *) p) = size;
        p += 16;
    }
    
    return (void *) p;
}

void *krealloc(void *buffer, int size)
{
    void *res = NULL;
    if (!buffer) return kmalloc(size); // buffer为NULL，则realloc相当于malloc
    if (!size) { // size为NULL，则realloc相当于free
        kfree(buffer);
        return NULL;
    }
    // 否则实现扩容
    res = kmalloc(size); // 分配新的缓冲区
    memcpy(res, buffer, size); // 将原缓冲区内容复制过去
    kfree(buffer); // 释放原缓冲区
    return res; // 返回新缓冲区
}

void kfree(void *p)
{
    char *q = (char *) p;
    int size = 0;
    if (q) {
        q -= 16;
        size = *((int *) q);
    }
    memman_t *memman = (memman_t *) MEMMAN_ADDR;
    memman_free(memman, (uint32_t) q, size + 16);
    p = NULL;
    return;
}


uint32_t init_mem(void)
/*初始化内存管理*/
{
	uint32_t memtotal=memtest(0x00400000, 0xbfffffff);


	memman_t *man=(memman_t *)MEMMAN_ADDR;
	memman_init(man);

	memman_free(man, 0x400000, memtotal - 0x400000);
	return memtotal;
}

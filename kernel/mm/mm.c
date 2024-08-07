/*
mm.c
内存管理程序
Copyright W24 Studio 
*/

#include <mm.h>
#include <regctl.h>
#include <stdint.h>
#include <stddef.h>

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
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

uint32_t memman_alloc(memman_t *man, uint32_t size)
/* 内部使用分配空间*/
{
	uint32_t i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];
				}
			}
			return a;
		}
	}
	return 0;
}

int memman_free(memman_t *man, uint32_t addr, uint32_t size)
/* 内部使用释放空间*/
{
	uint32_t i, j;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	if (i > 0) {
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			man->free[i - 1].size += size;
			if (i < man->frees) {
				if (addr + size == man->free[i].addr) {
					man->free[i - 1].size += man->free[i].size;
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];
					}
				}
			}
			return 0;
		}
	}
	if (i < man->frees) {
		if (addr + size == man->free[i].addr) {
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	if (man->frees < MEMMAN_FREES) {
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	man->losts++;
	man->lostsize += size;
	return -1;
}

void *malloc(uint32_t size)
/*分配空间*/
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

void free(void *p)
/*释放空间*/
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
	uint32_t memtotal=memtotal=memtest(0x00400000, 0xbfffffff);

	memman_t *man=(memman_t *)MEMMAN_ADDR;
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;


	
	return memtotal;
}

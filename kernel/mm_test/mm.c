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
#include <list.h>

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
	return 0;
}


#define BUF_COUNT 4 // 堆内存缓存页数量

static arena_descriptor_t descriptors[DESC_COUNT];

// arena 初始化
void arena_init()
{
    u32 block_size = 16;
    for (size_t i = 0; i < DESC_COUNT; i++)
    {
        arena_descriptor_t *desc = &descriptors[i];
        desc->block_size = block_size;
        desc->total_block = (PAGE_SIZE - sizeof(arena_t)) / block_size;
        desc->page_count = 0;
        list_init(&desc->free_list);
        block_size <<= 1; // block *= 2;
    }
}

// 获得 arena 第 idx 块内存指针
static void *get_arena_block(arena_t *arena, u32 idx)
{
    //assert(arena->desc->total_block > idx);
    void *addr = (void *)(arena + 1);
    u32 gap = idx * arena->desc->block_size;
    return addr + gap;
}

static arena_t *get_block_arena(block_t *block)
{
    return (arena_t *)((u32)block & 0xfffff000);
}

void *kmalloc(size_t size)
{
    arena_descriptor_t *desc = NULL;
    arena_t *arena;
    block_t *block;
    char *addr;

    if (size > 1024)
    {
        u32 asize = size + sizeof(arena_t);
        u32 count = div_round_up(asize, PAGE_SIZE);

        arena = (arena_t *)kvalloc(count);
        memset(arena, 0, count * PAGE_SIZE);
        arena->large = true;
        arena->count = count;
        arena->desc = NULL;
        arena->magic = 0x114514;

        addr = (char *)((u32)arena + sizeof(arena_t));
        return addr;
    }

    for (size_t i = 0; i < DESC_COUNT; i++)
    {
        desc = &descriptors[i];
        if (desc->block_size >= size)
            break;
    }

    //assert(desc != NULL);

    if (list_empty(&desc->free_list))
    {
        arena = (arena_t *)kvalloc(1);
        memset(arena, 0, PAGE_SIZE);

        desc->page_count++;

        arena->desc = desc;
        arena->large = false;
        arena->count = desc->total_block;
        arena->magic = 0x114514;

        for (size_t i = 0; i < desc->total_block; i++)
        {
            block = get_arena_block(arena, i);
            //assert(!list_search(&arena->desc->free_list, block));
            list_push(&arena->desc->free_list, block);
            //assert(list_search(&arena->desc->free_list, block));
        }
    }

    block = list_pop(&desc->free_list);

    arena = get_block_arena(block);
    //assert(arena->magic == 0x114514 && !arena->large);

    // memset(block, 0, desc->block_size);

    arena->count--;

    return block;
}

void kfree(void *ptr)
{
    //assert(ptr);

    block_t *block = (block_t *)ptr;
    arena_t *arena = get_block_arena(block);

    //assert(arena->large == 1 || arena->large == 0);
    //assert(arena->magic == 0x114514);

    if (arena->large)
    {
        kvfree((u32)arena, arena->count);
        return;
    }

    list_push(&arena->desc->free_list, block);
    arena->count++;

    if (arena->count == arena->desc->total_block && arena->desc->page_count > BUF_COUNT)
    {
        for (size_t i = 0; i < arena->desc->total_block; i++)
        {
            block = get_arena_block(arena, i);
            //assert(list_search(&arena->desc->free_list, block));
            list_remove(block);
            //assert(!list_search(&arena->desc->free_list, block));
        }
        arena->desc->page_count--;
        //assert(arena->desc->page_count >= BUF_COUNT);

        kvfree((u32)arena, 1);
    }
}

void *malloc(uint32_t size)
{
    return kmalloc(size);
}

void free(void *ptr)
{
    kfree(ptr);
}

uint32_t init_mem(void)
/*初始化内存管理*/
{
	uint32_t memtotal=memtest(0x00400000, 0xbfffffff);
	
    arena_init();
    memory_init_sub();

	return memtotal;
}

/*
page.c
分页式内存管理程序
Copyright W24 Studio 
*/

#include <mm.h>
#include <stdint.h>
#include <stddef.h>
#include <regctl.h>
#include <binfo.h>
#include <macro.h>

// 计算 num 分成 size 的数量
u32 div_round_up(u32 num, u32 size)
{
    return (num + size - 1) / size;
}

#define ceil(num,size) div_round_up(num,size);

/**
 * 获取 ards 信息
 * 
 * @param ptr ards表 基址
 * @return void
 */
void get_ards_info(ptr_t ptr) {
    size_t count = *((size_t *)ptr);    // ards表数量
    ards_t *ards = (ards_t *)(ptr + 4);

    for (size_t i = 0; i < count; i++, ards++) {
        if (ards->type == 1 && ards->size > mi.free_size) {
            mi.free_base = ards->base;
            mi.free_size = ards->size;
            mi.total_size += ards->size;
        }
    }

    mi.total_pages = mi.total_size / PAGE_SIZE;
    mi.free_pages = mi.free_size / PAGE_SIZE;

    //printkf(binfo->vram, vbe->resx, 0, 0xffaaee00, "============== Memory Info ==============\n");
    //printf("Total Size:  %d MB\n", ceil(mi.total_size, MB_SIZE));
    //printf("Total Pages: %d\n", mi.total_pages);
    //printf("Free Base:   0x%x\n", mi.free_base);
    //printf("Free Size:   %d MB\n", ceil(mi.free_size, MB_SIZE));
    //printf("Free Pages:  %d\n", mi.free_pages);
}

/**
 * 物理内存管理 初始化
 * 
 * @param void
 * @return void
 */
void phy_init(void) {
    phy_map = (u8 *)mi.free_base;
    phy_map_pages = ceil(mi.total_pages, PAGE_SIZE);

    // 清空物理内存映射表
    memset((void *)phy_map, 0, phy_map_pages * PAGE_SIZE);

    phy_alloc_idx = IDX(mi.free_base) + phy_map_pages;
    for (size_t i = 0; i < phy_alloc_idx; i++) {
        phy_map[i] = 1;
    }
    
    mi.free_pages -= phy_map_pages;

    //printkf(binfo->vram, vbe->resx, 0, 0xffaaee00, "============== Memory Phy Map Info ==============\n");
    //printf("Phy Map:        0x%X\n", phy_map);
    //printf("Phy Map Pages:  %d\n", phy_map_pages);
    //printf("Phy Alloc Base: 0x%x\n", ADDR(phy_alloc_idx));
}

/**
 * 分配1页物理页
 * 
 * @param void
 * @return 物理页基址
 */
ptr_t palloc(void) {
    for (size_t i = phy_alloc_idx; i < mi.total_pages; i++) {
        if (phy_map[i] == 0) {
            ptr_t ptr = ADDR(i);
            phy_map[i] = 1;  // 标记为 已分配
            return ptr;
        }
    }
}

/**
 * 释放1页物理页
 * 
 * @param ptr 物理页基址
 * @return void
 */
void pfree(ptr_t ptr) {
    size_t idx = IDX(ptr); // 获取物理页索引

    phy_map[idx] = 0;  // 标记为 未分配
    mi.free_pages++;
}


/**
 * 开启分页
 * 
 * @param void
 * @return void
 */
void enable_paging(void)
{
    uint32_t cr0=load_cr0();
    cr0|=0x80000000;
    store_cr0(cr0);
}

/**
 * 获取页目录基址
 * 
 * @param void
 * @return 页目录基址
 */
entry_t *get_pde(void) {
    return (entry_t *)(0xfffff000);
}

/**
 * 设置页项
 * 
 * @param entry 页项
 * @param idx 索引
 * @return void
 */
void set_entry(entry_t *entry, u32 idx)
{
    *(u32 *)entry = 0; // 清空项

    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = idx;
}

/**
 * 虚拟内存映射
 * 
 * @param void
 * @return void
 */
void vir_mapping(void) {

    struct BOOTINFO *binfo=(struct BOOTINFO *)ADR_BOOTINFO;
    

    // 页目录项
    entry_t *pd = (entry_t *)PAGE_DIR_BASE;
    // 页表项
    entry_t *pte = (entry_t *)(PAGE_DIR_BASE + PAGE_SIZE);

    size_t aidx = 0;
    size_t frame_idx = IDX(binfo->vram);

    // 初始化 页表基址数组
    for (size_t idx = 0; idx < 6; idx++) {
        kernel_pt[idx] = (u32)pte;
        pte = (entry_t *)((u32)pte + PAGE_SIZE);
    }
    for (size_t idx = 0; idx < 4; idx++) {
        frame_pt[idx] = (u32)pte;
        pte = (entry_t *)((u32)pte + PAGE_SIZE);
    }

    // 清空页目录
    memset(pd, 0, PAGE_SIZE);

    // 映射内核区
    for (size_t pidx = 0; pidx < (sizeof(kernel_pt) / 4); pidx++) {
        pte = (entry_t *)kernel_pt[pidx];
        memset(pte, 0, PAGE_SIZE);  // 清空页表

        // 设置 页目录
        set_entry(&pd[pidx], IDX(pte));

        for (size_t idx = 0; idx < 1024; idx++, aidx++) {
            set_entry(&pte[idx], aidx);
            phy_map[aidx] = 1;
        }
    }
    
    u32 fpde_idx = DIDX(binfo->vram); // 帧缓冲页目录项索引

    // 映射帧缓冲
    for (size_t pidx = 0; pidx < (sizeof(frame_pt) / 4); pidx++, fpde_idx++) {
        pte = (entry_t *)frame_pt[pidx];
        memset(pte, 0, PAGE_SIZE);  // 清空页表

        // 设置 页目录
        set_entry(&pd[fpde_idx], IDX(pte));

        for (size_t idx = 0; idx < 1024; idx++, frame_idx++) {
            set_entry(&pte[idx], frame_idx);
            phy_map[frame_idx] = 1;
        }
    }

    set_entry(&pd[1023], IDX(PAGE_DIR_BASE));  // 映射页目录

    // 设置 cr3寄存器
    set_cr3((ptr_t)PAGE_DIR_BASE);
    
    // 开启分页
    enable_paging();
    for(;;);
}

/**
 * 分配内核虚拟页
 * 
 * @param cnt 页数
 * @return 内核虚拟页基址
 */
void *kvalloc(size_t cnt) {
    u32 idx = bitmap_scan(&kernel_map, cnt);

    if (idx == -1) {
        return NULL;
    }

    void *ptr = (void *)(ADDR(idx));
    return ptr;
}

/**
 * 释放内核虚拟页
 * 
 * @param ptr 内核虚拟页基址
 * @param cnt 页数
 * @return void
 */
void kvfree(void *ptr, size_t cnt) {
    u32 idx = IDX(ptr);

    // 重置位值
    for (size_t i = 0; i < cnt; i++) {
        bitmap_set(&kernel_map, idx + i, false);
    }
}

/**
 * 内存管理初始化
 * 
 * @param void
 * @return void
 */
void memory_init_sub(void) {
    //printkf(binfo->vram, vbe->resx, 0, 0xffffff00, "[ INFO ] Memory Init Starting...\n");    

    get_ards_info(ARDS_BASE);  // 获取 ards 信息
    phy_init();     // 物理内存管理 初始化

    // 初始化 内核虚拟内存位图
    kernel_map_size = (IDX(24 * MB_SIZE) - IDX(mi.free_base)) / 8;
    bitmap_init(&kernel_map, (u8 *)KERNEL_MAP_BASE, kernel_map_size, IDX(mi.free_base));
    bitmap_scan(&kernel_map, phy_map_pages);

    vir_mapping();  // 虚拟内存映射
    arena_init();   // arena 初始化
    
    //printkf(binfo->vram, vbe->resx, 0, 0xffffff00, "[ INFO ] End...\n\n");  
}
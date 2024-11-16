/*
mm.h
内存管理程序头文件
Copyright W24 Studio 
*/

#ifndef MM_H
#define MM_H
#include <stdint.h>
#include <list.h>

typedef u32 ptr_t;

// 位图
typedef struct {
    u8 *buf;    // 缓冲区
    u32 size;   // 缓冲区大小
    u32 off;    // 起始偏移量
} bitmap_t;

void bitmap_init(bitmap_t *bitmap, u8 *buf, u32 size, u32 off);
bool bitmap_test(bitmap_t *bitmap, u32 idx);
void bitmap_set(bitmap_t *bitmap, u32 idx, u32 value);
u32 bitmap_scan(bitmap_t *bitmap, u32 cnt);


// ards表 基址
#define ARDS_BASE 0x5000
// 页目录物理基址
#define PAGE_DIR_BASE 0x1000
// 内核虚拟内存位图基址
#define KERNEL_MAP_BASE 0xC000

// 页 大小
#define PAGE_SIZE 4096
// MB 大小
#define MB_SIZE (1024 * 1024)

// 获取 addr 对应的 索引
#define IDX(addr) ((u32)addr >> 12)
// 获取 addr 对应的 地址
#define ADDR(idx) ((u32)idx << 12)
// 获取 addr 对应的 页目录索引
#define DIDX(addr) (((u32)addr >> 22) & 0x3ff)

// ards表
typedef struct {
    u64 base;   // 起始地址
    u64 size;   // 大小
    u32 type;   // 类型
} ards_t;

// 内存信息表
typedef struct {
    u32 total_size;     // 总内存大小
    u32 total_pages;    // 总页数
    u32 free_base;      // 可用内存基址
    u32 free_size;      // 可用内存大小
    u32 free_pages;     // 空闲页数
} mem_info_t;

// 页项
typedef struct _packed {
    u8 present : 1;  // 存在位     (1：内存中 0：磁盘上)
    u8 write : 1;    // 读写权限   (0：只读 1：可读可写)
    u8 user : 1;     // 用户权限   (1：所有人 0：超级用户)
    u8 pwt : 1;      // 页写模式   (1:直写模式；0:回写模式)
    u8 pcd : 1;      // 禁止页缓冲
    u8 accessed : 1; // 访问位
    u8 dirty : 1;    // 脏页(页缓冲被写过)
    u8 pat : 1;      // 页大小     (4K / 4M)
    u8 global : 1;   // 全局位
    u8 ignored : 3;  // 可用位
    u32 index : 20;  // 页项索引
} entry_t;

static mem_info_t mi;  // 内存信息

static u8 *phy_map;               // 物理内存映射表
static size_t phy_map_pages;      // 物理内存映射表页数
static size_t phy_alloc_idx;      // 可分配物理页 起始索引

static bitmap_t kernel_map;       // 内核虚拟内存位图
static size_t kernel_map_size;    // 内核虚拟内存位图页数

static u32 kernel_pt[6];   // 内核页表基址数组
static u32 frame_pt[4];    // 帧缓冲页表基址数组

void get_ards_info(ptr_t ptr);
void phy_init(void);
ptr_t palloc(void);
void pfree(ptr_t ptr);
void enable_paging(void);
entry_t *get_pde(void);
void set_entry(entry_t *entry, u32 idx);
void vir_mapping(void);
void *kvalloc(size_t cnt);
void kvfree(void *ptr, size_t cnt);
void memory_init_sub(void);


#define DESC_COUNT 7

typedef list_node_t block_t; // 内存块

// 内存描述符
typedef struct arena_descriptor_t
{
    u32 total_block;  // 一页内存分成了多少块
    u32 block_size;   // 块大小
    int page_count;   // 空闲页数量
    list_t free_list; // 空闲列表
} arena_descriptor_t;

// 一页或多页内存
typedef struct arena_t
{
    arena_descriptor_t *desc; // 该 arena 的描述符
    u32 count;                // 当前剩余多少块 或 页数
    u32 large;                // 表示是不是超过 1024 字节
    u32 magic;                // 魔数
} arena_t;

void *malloc(uint32_t size);
void free(void *ptr);

uint32_t memtest(uint32_t start, uint32_t end);
uint32_t free_space_total(void);

#endif
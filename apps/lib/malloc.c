//会被stdlib.c包含
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <napi/memory.h>

typedef char ALIGN[16];

typedef union header {
    struct {
        uint32_t size;
        uint32_t is_free;
        union header *next;
    } s;
    ALIGN stub;
} header_t;

static header_t *head, *tail;

// 寻找一个符合条件的指定大小的空闲内存块
static header_t *get_free_block(uint32_t size)
{
    header_t *curr = head; // 从头开始
    while (curr) {
        if (curr->s.is_free && curr->s.size >= size) return curr; // 空闲，并且大小也满足条件，直接返回
        curr = curr->s.next; // 下一位
    }
    return NULL; // 找不到
}

void *malloc(uint32_t size)
{
    uint32_t total_size;
    void *block;
    header_t *header;
    if (!size) return NULL; // size == 0，自然不用返回
    header = get_free_block(size);
    if (header) { // 找到了对应的header！
        header->s.is_free = 0;
        return (void *) (header + 1);
        // header + 1，相当于把header的值在指针上后移了一个header_t，从而在返回的内存中不存在覆写header的现象
    }
    // 否则，申请内存
    total_size = sizeof(header_t) + size; // 需要一处放header的空间
    block = napi_heap_resize(total_size); // sbrk，申请total_size大小内存
    if (block == (void *) -1) return NULL; // 没有足够的内存，返回NULL
    // 申请成功！
    header = block; // 初始化header
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if (!head) head = header; // 第一个还是空的，直接设为header
    if (tail) tail->s.next = header; // 有最后一个，把最后一个的next指向header
    tail = header; // header荣登最后一个
    return (void *) (header + 1); // 同上
}

void free(void *block)
{
    header_t *header, *tmp;
    if (!block) return; // free(NULL)，有什么用捏
    header = (header_t *) block - 1; // 减去一个header_t的大小，刚好指向header_t

    if ((char *) block + header->s.size == napi_heap_resize(0)) { // 正好在堆末尾
        if (head == tail) head = tail = NULL; // 只有一个内存块，全部清空
        else {
            // 遍历整个内存块链表，找到对应的内存块，并把它从链表中删除
            tmp = head;
            while (tmp) {
                // 如果内存在堆末尾，那这个块肯定也在链表末尾
                if (tmp->s.next == tail) { // 下一个就是原本末尾
                    tmp->s.next = NULL; // 踢掉
                    tail = tmp; // 末尾位置顶替
                }
                tmp = tmp->s.next; // 下一个
            }
        }
        // 释放这一块内存
        napi_heap_resize(0 - sizeof(header_t) - header->s.size);
        return;
    }
    // 否则，设置为free
    header->s.is_free = 1;
}

void *calloc(size_t num, size_t size)
{
    if(num==0 || size==0)
    {
        return NULL;
    }
    uint32_t total_size=num*size;
    if(size!=0&&num>SIZE_MAX/size)
    {
        return NULL;
    }
    void *p=malloc(total_size);
    if(!p)
    {
        return NULL;
    }
    memset(p,0,total_size);
    return p;
}

void *realloc(void *ptr, size_t new_size) {
    if (!ptr) {
        return malloc(new_size);
    }
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    
    header_t *header = (header_t *)ptr - 1;
    uint32_t old_usable_size = header->s.size;
    
    // 如果新大小小于等于原可用大小，直接返回
    if (new_size <= old_usable_size) {
        return ptr;
    }

    
    // 分配新内存
    void *new_ptr = malloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    // 复制数据
    memcpy(new_ptr, ptr, old_usable_size);
    
    // 释放旧内存
    free(ptr);
    
    return new_ptr;
}
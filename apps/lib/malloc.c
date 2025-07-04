#include <unistd.h>
#include <stddef.h>

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
    block = sbrk(total_size); // sbrk，申请total_size大小内存
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

    if ((char *) block + header->s.size == sbrk(0)) { // 正好在堆末尾
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
        sbrk(0 - sizeof(header_t) - header->s.size);
        return;
    }
    // 否则，设置为free
    header->s.is_free = 1;
}
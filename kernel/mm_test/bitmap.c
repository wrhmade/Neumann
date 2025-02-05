/*
bitmap.c
内存位图管理程序
Copyright W24 Studio 
*/

#include <mm.h>
#include <stddef.h>
#include <string.h>

/**
 * 位图初始化
 * 
 * @param bitmap 位图
 * @param buf 位图缓冲区
 * @param size 位图缓冲区大小
 * @param off 位图起始偏移量
 * 
 * @return void
 */
void bitmap_init(bitmap_t *bitmap, u8 *buf, u32 size, u32 off) {
    bitmap->buf = buf;
    bitmap->size = size;
    bitmap->off = off;

    memset(bitmap->buf, 0, size);
}

/**
 * 位图 检测位值
 * 
 * @param bitmap 位图
 * @param idx 索引
 * 
 * @return 位值
 */
bool bitmap_test(bitmap_t *bitmap, u32 idx) {
    // 索引超出范围
    if (idx < bitmap->off)
        return false;

    u32 index = idx - bitmap->off;

    u32 byte = index / 8;  // 字节索引
    u32 bit = index % 8;   // 位索引

    // 字节索引超出范围
    if (byte >= bitmap->size)
        return false;

    return (bitmap->buf[byte] & (1 << bit));
}

/**
 * 位图 设置位值
 * 
 * @param bitmap 位图
 * @param idx 索引
 * @param value 位值
 * 
 * @return void
 */
void bitmap_set(bitmap_t *bitmap, u32 idx, u32 value) {
    // 索引超出范围
    if (idx < bitmap->off)
        return;

    u32 index = idx - bitmap->off;

    u32 byte = index / 8;  // 字节索引
    u32 bit = index % 8;   // 位索引

    // 字节索引超出范围
    if (byte >= bitmap->size)
        return;

    if (value) {
        // 置1
        bitmap->buf[byte] |= (1 << bit);
    } else {
        // 置0
        bitmap->buf[byte] &= ~(1 << bit);
    }
}

/**
 * 位图 查找空闲位
 * 
 * @param bitmap 位图
 * @param cnt 空闲位数
 * 
 * @return 起始索引
 */
u32 bitmap_scan(bitmap_t *bitmap, u32 cnt) {
    u32 start = -1;     // 起始索引
    u32 count = 0;      // 空闲位计数
    u32 nxt_bit = 0;    // 下一个空闲位
    u32 bits = bitmap->size * 8; // 位图位数

    while (bits-- > 0) {
        // 检测空闲位位值
        if (!bitmap_test(bitmap, bitmap->off + nxt_bit)) {
            count++;
        } else {
            count = 0;
        }

        nxt_bit++;
        
        // 找到足够空闲位
        if (count == cnt) {
            start = bitmap->off + (nxt_bit - cnt);
            break;
        }
    }

    if (start == -1) {
        return -1;
    }

    // 设置位值
    for (size_t i = 0; i < count; i++) {
        bitmap_set(bitmap, start + i, true);
    }
    
    return start;
}
/*
hd.c
IDE硬盘驱动程序
Copyright W24 Studio 
*/

#include <hd.h>
#include <io.h>
#include <stdint.h>
#include <mm.h>

static int hd_size_cache = 0;

// 等待磁盘，直到它就绪
static void wait_disk_ready()
{
    while (1) {
        uint8_t data = io_in8(0x1f7); // 输入时，0x1f7端口为主硬盘状态寄存器
        if ((data & 0x88) == 0x08) { // 第7位：硬盘忙，第3位：硬盘已经准备好
            // 提取第7位和第3位，判断是否为0x08，即硬盘不忙且已准备好
            return; // 等完了
        }
    }
}

// 选择要操作扇区
static void select_sector(int lba)
{
    // 第一步：向0x1f2端口指定要读取扇区数
    // 输出时，0x1f2端口为操作扇区数
    io_out8(0x1f2, 1);
    // 第二步：存入写入地址
    // 0x1f3~0x1f5：LBA的低中高8位
    // 0x1f6：REG_DEVICE，Drive | Head | LBA (24~27位)
    // 在实际操作中，只有一个硬盘，Drive | Head = 0xe0
    io_out8(0x1f3, lba);
    io_out8(0x1f4, lba >> 8);
    io_out8(0x1f5, lba >> 16);
    io_out8(0x1f6, (((lba >> 24) & 0x0f) | 0xe0));
}

// 读取一个扇区
static void read_a_sector(int lba, uint32_t buffer)
{
    while (io_in8(0x1f7) & 0x80); // 等硬盘不忙了再发送命令，具体意义见wait_disk_ready
    select_sector(lba); // 第二步：设置读写扇区
    io_out8(0x1f7, 0x20); // 第三步：宣布要读扇区
    // 0x1f7在被写入时为REG_COMMAND，写入读写命令
    wait_disk_ready(); // 第四步：检测硬盘状态，直到硬盘就绪
    // 第五步：从0x1f0读取数据
    // 0x1f0被读写时为REG_DATA，读出或写入数据
    for (int i = 0; i < 256; i++) {
        // 每次硬盘会发送2个字节数据
        uint16_t data = io_in16(0x1f0);
        *((uint16_t *) buffer) = data; // 存入buf
        buffer += 2;
    }
}

// 写入一个扇区
// 写入与读取基本一致，仅有的不同之处是写入的命令和写数据的操作
static void write_a_sector(int lba, uint32_t buffer)
{
    while (io_in8(0x1f7) & 0x80); // 等硬盘不忙了再发送命令，具体意义见wait_disk_ready
    select_sector(lba); // 第二步：设置读写扇区
    io_out8(0x1f7, 0x30); // 第三步：宣布要写扇区
    // 0x1f7在被写入时为REG_COMMAND，写入读写命令
    wait_disk_ready(); // 第四步：检测硬盘状态，直到硬盘就绪
    // 第五步：从0x1f0读取数据
    // 0x1f0被读写时为REG_DATA，读出或写入数据
    for (int i = 0; i < 256; i++) {
        // 每次硬盘会发送2个字节数据
        uint16_t data = *((uint16_t *) buffer); // 读取数据
        io_out16(0x1f0, data); // 写入端口
        buffer += 2;
    }
}

// 读取硬盘
static void read_disk(int lba, int sec_cnt, uint32_t buffer)
{
    for (int i = 0; i < sec_cnt; i++) {
        read_a_sector(lba, buffer); // 一次读一个扇区
        lba++; // 下一个扇区
        buffer += 512; // buffer也要指向下一个扇区
    }
}

// 写入硬盘
static void write_disk(int lba, int sec_cnt, uint32_t buffer)
{
    for (int i = 0; i < sec_cnt; i++) {
        write_a_sector(lba, buffer); // 一次写一个扇区
        lba++; // 下一个扇区
        buffer += 512; // buffer也要指向下一个扇区
    }
}

// 包装
void hd_read(int lba, int sec_cnt, void *buffer)
{
    read_disk(lba, sec_cnt, (uint32_t) buffer);
}

void hd_write(int lba, int sec_cnt, void *buffer)
{
    write_disk(lba, sec_cnt, (uint32_t) buffer);
}

int get_hd_sects()
{
    if (hd_size_cache) return hd_size_cache;
    while (io_in8(0x1f7) & 0x80); // 等硬盘不忙了再发送命令，具体意义见wait_disk_ready
    io_out16(0x1f6, 0x00);
    io_out16(0x1f7, 0xec); // IDENTIFY 命令
    wait_disk_ready();
    uint16_t *hdinfo = (uint16_t *) malloc(512);
    char *buffer = (char *) hdinfo;
    for (int i = 0; i < 256; i++) {
        // 每次硬盘会发送2个字节数据
        uint16_t data = io_in16(0x1f0);
        *((uint16_t *) buffer) = data; // 存入buf
        buffer += 2;
    }
    int sectors = ((int) hdinfo[61] << 16) + hdinfo[60];
    free(hdinfo);
    return (hd_size_cache = sectors);
}

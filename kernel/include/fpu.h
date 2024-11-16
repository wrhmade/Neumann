/*
fpu.h
浮点处理器驱动程序头文件
Copyright W24 Studio 
*/

#ifndef FPU_H
#define FPU_H
#include <stddef.h>
typedef struct TASK task_t;
enum
{
    CR0_PE = 1 << 0,  // Protection Enable 启用保护模式
    CR0_MP = 1 << 1,  // Monitor Coprocessor
    CR0_EM = 1 << 2,  // Emulation 启用模拟，表示没有 FPU
    CR0_TS = 1 << 3,  // Task Switch 任务切换，延迟保存浮点环境
    CR0_ET = 1 << 4,  // Extension Type 保留
    CR0_NE = 1 << 5,  // Numeric Error 启用内部浮点错误报告
    CR0_WP = 1 << 16, // Write Protect 写保护（禁止超级用户写入只读页）帮助写时复制
    CR0_AM = 1 << 18, // Alignment Mask 对齐掩码
    CR0_NW = 1 << 29, // Not Write-Through 不是直写
    CR0_CD = 1 << 30, // Cache Disable 禁用内存缓冲
    CR0_PG = 1 << 31, // Paging 启用分页
};

// Intel® 64 and IA-32 Architectures Software Developer's Manual
// Figure 8-9. Protected Mode x87 FPU State Image in Memory, 32-Bit Format

typedef struct fpu_t
{
    uint16_t control;
    uint16_t RESERVED;
    uint16_t status;
    uint16_t RESERVED1;
    uint16_t tag;
    uint16_t RESERVED2;
    uint32_t fip0;
    uint32_t fop0;
    uint32_t fdp0;
    uint32_t fdp1;
    uint8_t regs[80];
} __attribute__((packed))fpu_t;

bool fpu_check();
void fpu_disable(task_t *task);
void fpu_enable(task_t *task);
bool init_fpu();
#endif
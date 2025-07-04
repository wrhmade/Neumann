/*
pci.h
PCI设备驱动头文件
Copyright W24 Studio 
*/

#ifndef PCI_H
#define PCI_H
#include <stdint.h>
#include <list.h>
#include <console.h>
#define PCI_CONF_VENDOR 0X0   // 厂商
#define PCI_CONF_DEVICE 0X2   // 设备
#define PCI_CONF_COMMAND 0x4  // 命令
#define PCI_CONF_STATUS 0x6   // 状态
#define PCI_CONF_REVISION 0x8 //
#define PCI_CONF_BASE_ADDR0 0x10
#define PCI_CONF_BASE_ADDR1 0x14
#define PCI_CONF_BASE_ADDR2 0x18
#define PCI_CONF_BASE_ADDR3 0x1C
#define PCI_CONF_BASE_ADDR4 0x20
#define PCI_CONF_BASE_ADDR5 0x24
#define PCI_CONF_INTERRUPT 0x3C

#define PCI_CLASS_MASK 0xFF0000
#define PCI_SUBCLASS_MASK 0xFFFF00

#define PCI_CLASS_STORAGE_IDE 0x010100

#define PCI_BAR_TYPE_MEM 0
#define PCI_BAR_TYPE_IO 1

#define PCI_BAR_IO_MASK (~0x3)
#define PCI_BAR_MEM_MASK (~0xF)

#define PCI_COMMAND_IO 0x0001          // Enable response in I/O space
#define PCI_COMMAND_MEMORY 0x0002      // Enable response in Memory space
#define PCI_COMMAND_MASTER 0x0004      // Enable bus mastering
#define PCI_COMMAND_SPECIAL 0x0008     // Enable response to special cycles
#define PCI_COMMAND_INVALIDATE 0x0010  // Use memory write and invalidate
#define PCI_COMMAND_VGA_PALETTE 0x0020 // Enable palette snooping
#define PCI_COMMAND_PARITY 0x0040      // Enable parity checking
#define PCI_COMMAND_WAIT 0x0080        // Enable address/data stepping
#define PCI_COMMAND_SERR 0x0100        // Enable SERR/
#define PCI_COMMAND_FAST_BACK 0x0200   // Enable back-to-back writes

#define PCI_STATUS_CAP_LIST 0x010    // Support Capability List
#define PCI_STATUS_66MHZ 0x020       // Support 66 Mhz PCI 2.1 bus
#define PCI_STATUS_UDF 0x040         // Support User Definable Features [obsolete]
#define PCI_STATUS_FAST_BACK 0x080   // Accept fast-back to back
#define PCI_STATUS_PARITY 0x100      // Detected parity error
#define PCI_STATUS_DEVSEL_MASK 0x600 // DEVSEL timing
#define PCI_STATUS_DEVSEL_FAST 0x000
#define PCI_STATUS_DEVSEL_MEDIUM 0x200
#define PCI_STATUS_DEVSEL_SLOW 0x400

typedef struct pci_addr_t
{
    u8 RESERVED : 2; // 最低位
    u8 offset : 6;   // 偏移
    u8 function : 3; // 功能号
    u8 device : 5;   // 设备号
    u8 bus;          // 总线号
    u8 RESERVED2 : 7; // 保留
    u8 enable : 1;   // 地址有效
} __attribute__((packed)) pci_addr_t;

typedef struct pci_bar_t
{
    u32 iobase;
    u32 size;
} pci_bar_t;

typedef struct pci_device_t
{
    u8 bus;
    u8 dev;
    u8 func;
    u16 vendorid;
    u16 deviceid;
    u8 revision;
    u32 classcode;
} pci_device_t;

#define TIMELESS -1 // 无限时间
typedef int err_t; // 错误类型
enum
{
    EOK = 0,           // 没错
    EPERM = 1,         // 操作没有许可
    ENOENT = 2,        // 文件或目录不存在
    ESRCH = 3,         // 指定的进程不存在
    EINTR = 4,         // 中断的函数调用
    EIO = 5,           // 输入输出错误
    ENXIO = 6,         // 指定设备或地址不存在
    E2BIG = 7,         // 参数列表太长
    ENOEXEC = 8,       // 执行程序格式错误
    EBADF = 9,         // 文件描述符错误
    ECHILD = 10,       // 子进程不存在
    EAGAIN = 11,       // 资源暂时不可用
    ENOMEM = 12,       // 内存不足
    EACCES = 13,       // 没有许可权限
    EFAULT = 14,       // 地址错
    ENOTBLK = 15,      // 不是块设备文件
    EBUSY = 16,        // 资源正忙
    EEXIST = 17,       // 文件已存在
    EXDEV = 18,        // 非法连接
    ENODEV = 19,       // 设备不存在
    ENOTDIR = 20,      // 不是目录文件
    EISDIR = 21,       // 是目录文件
    EINVAL = 22,       // 参数无效
    ENFILE = 23,       // 系统打开文件数太多
    EMFILE = 24,       // 打开文件数太多
    ENOTTY = 25,       // 不恰当的 IO 控制操作(没有 tty 终端)
    ETXTBSY = 26,      // 不再使用
    EFBIG = 27,        // 文件太大
    ENOSPC = 28,       // 设备已满（设备已经没有空间）
    ESPIPE = 29,       // 无效的文件指针重定位
    EROFS = 30,        // 文件系统只读
    EMLINK = 31,       // 连接太多
    EPIPE = 32,        // 管道错
    EDOM = 33,         // 域(domain)出错
    ERANGE = 34,       // 结果太大
    EDEADLK = 35,      // 避免资源死锁
    ENAMETOOLONG = 36, // 文件名太长
    ENOLCK = 37,       // 没有锁定可用
    ENOSYS = 38,       // 功能还没有实现
    ENOTEMPTY = 39,    // 目录不空
    ETIME = 62,        // 超时
    EFSUNK,            // 文件系统未知
    EMSGSIZE = 90,     // 消息过长
    ERROR = 99,        // 一般错误
    EEOF,              // 读写文件结束

    // 网络错误
    EADDR,     // 地址错误
    EPROTO,    // 协议错误
    EOPTION,   // 选项错误
    EFRAG,     // 分片错误
    ESOCKET,   // 套接字错误
    EOCCUPIED, // 被占用
    ENOTCONN,  // 没有连接
    ERESET,    // 连接重置
    ECHKSUM,   // 校验和错误

    // 错误数量，应该在枚举的最后
    ENUM,
};

u32 pci_inl(u8 bus, u8 dev, u8 func, u8 addr);
void pci_outl(u8 bus, u8 dev, u8 func, u8 addr, u32 value);

err_t pci_find_bar(pci_device_t *device, pci_bar_t *bar, int type);
u8 pci_interrupt(pci_device_t *device);

const char *pci_classname(u32 classcode);

pci_device_t *pci_find_device(u16 vendorid, u16 deviceid);
pci_device_t *pci_find_device_by_class(u32 classcode);
void pci_enable_busmastering(pci_device_t *device);
int count_pci_device();
void cmd_lspci(console_t *console);
#endif
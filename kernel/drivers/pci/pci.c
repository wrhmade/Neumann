/*
pci.c
PCI设备驱动
Copyright W24 Studio 
*/

#include <pci.h>
#include <stdint.h>
#include <mm.h>
#include <list.h>
#include <console.h>
#include <task.h>
#include <krnlcons.h>
#include <io.h>
#include <stdio.h>

#define PCI_CONF_ADDR 0xCF8
#define PCI_CONF_DATA 0xCFC

#define PCI_BAR_NR 6

#define PCI_ADDR(bus, dev, func, addr) ( \
    (u32)(0x80000000) |                  \
    ((bus & 0xff) << 16) |               \
    ((dev & 0x1f) << 11) |               \
    ((func & 0x7) << 8) |                \
    addr)

static list_t pci_device_list;

struct
{
	uint32_t classcode;
	char *name;
} pci_classnames[] = {
	{0x000000, "Non-VGA-Compatible Unclassified Device"},
	{0x000100, "VGA-Compatible Unclassified Device"},

	{0x010000, "SCSI Bus Controller"},
	{0x010100, "IDE Controller"},
	{0x010200, "Floppy Disk Controller"},
	{0x010300, "IPI Bus Controller"},
	{0x010400, "RAID Controller"},
	{0x010500, "ATA Controller"},
	{0x010600, "Serial ATA Controller"},
	{0x010700, "Serial Attached SCSI Controller"},
	{0x010800, "Non-Volatile Memory Controller"},
	{0x018000, "Other Mass Storage Controller"},

	{0x020000, "Ethernet Controller"},
	{0x020100, "Token Ring Controller"},
	{0x020200, "FDDI Controller"},
	{0x020300, "ATM Controller"},
	{0x020400, "ISDN Controller"},
	{0x020500, "WorldFip Controller"},
	{0x020600, "PICMG 2.14 Multi Computing Controller"},
	{0x020700, "Infiniband Controller"},
	{0x020800, "Fabric Controller"},
	{0x028000, "Other Network Controller"},

	{0x030000, "VGA Compatible Controller"},
	{0x030100, "XGA Controller"},
	{0x030200, "3D Controller (Not VGA-Compatible)"},
	{0x038000, "Other Display Controller"},

	{0x040000, "Multimedia Video Controller"},
	{0x040100, "Multimedia Audio Controller"},
	{0x040200, "Computer Telephony Device"},
	{0x040300, "Audio Device"},
	{0x048000, "Other Multimedia Controller"},

	{0x050000, "RAM Controller"},
	{0x050100, "Flash Controller"},
	{0x058000, "Other Memory Controller"},

	{0x060000, "Host Bridge"},
	{0x060100, "ISA Bridge"},
	{0x060200, "EISA Bridge"},
	{0x060300, "MCA Bridge"},
	{0x060400, "PCI-to-PCI Bridge"},
	{0x060500, "PCMCIA Bridge"},
	{0x060600, "NuBus Bridge"},
	{0x060700, "CardBus Bridge"},
	{0x060800, "RACEway Bridge"},
	{0x060900, "PCI-to-PCI Bridge"},
	{0x060A00, "InfiniBand-to-PCI Host Bridge"},
	{0x068000, "Other Bridge"},

	{0x070000, "Serial Controller"},
	{0x070100, "Parallel Controller"},
	{0x070200, "Multiport Serial Controller"},
	{0x070300, "Modem"},
	{0x070400, "IEEE 488.1/2 (GPIB) Controller"},
	{0x070500, "Smart Card Controller"},
	{0x078000, "Other Simple Communication Controller"},

	{0x080000, "PIC"},
	{0x080100, "DMA Controller"},
	{0x080200, "Timer"},
	{0x080300, "RTC Controller"},
	{0x080400, "PCI Hot-Plug Controller"},
	{0x080500, "SD Host controller"},
	{0x080600, "IOMMU"},
	{0x088000, "Other Base System Peripheral"},

	{0x090000, "Keyboard Controller"},
	{0x090100, "Digitizer Pen"},
	{0x090200, "Mouse Controller"},
	{0x090300, "Scanner Controller"},
	{0x090400, "Gameport Controller"},
	{0x098000, "Other Input Device Controller"},

	{0x0A0000, "Generic"},
	{0x0A8000, "Other Docking Station"},

	{0x0B0000, "386"},
	{0x0B0100, "486"},
	{0x0B0200, "Pentium"},
	{0x0B0300, "Pentium Pro"},
	{0x0B1000, "Alpha"},
	{0x0B2000, "PowerPC"},
	{0x0B3000, "MIPS"},
	{0x0B4000, "Co-Processor"},
	{0x0B8000, "Other Processor"},

	{0x0C0000, "FireWire (IEEE 1394) Controller"},
	{0x0C0100, "ACCESS Bus Controller"},
	{0x0C0200, "SSA"},
	{0x0C0300, "USB Controller"},
	{0x0C0400, "Fibre Channel"},
	{0x0C0500, "SMBus Controller"},
	{0x0C0600, "InfiniBand Controller"},
	{0x0C0700, "IPMI Interface"},
	{0x0C0800, "SERCOS Interface (IEC 61491)"},
	{0x0C0900, "CANbus Controller"},
	{0x0C8000, "Other Serial Bus Controller"},

	{0x0D0000, "iRDA Compatible Controlle"},
	{0x0D0100, "Consumer IR Controller"},
	{0x0D1000, "RF Controller"},
	{0x0D1100, "Bluetooth Controller"},
	{0x0D1200, "Broadband Controller"},
	{0x0D2000, "Ethernet Controller (802.1a)"},
	{0x0D2100, "Ethernet Controller (802.1b)"},
	{0x0D8000, "Other Wireless Controller"},

	{0x0E0000, "I20"},

	{0x0F0000, "Satellite TV Controller"},
	{0x0F0100, "Satellite Audio Controller"},
	{0x0F0300, "Satellite Voice Controller"},
	{0x0F0400, "Satellite Data Controller"},

	{0x100000, "Network and Computing Encrpytion/Decryption"},
	{0x101000, "Entertainment Encryption/Decryption"},
	{0x108000, "Other Encryption Controller"},

	{0x110000, "DPIO Modules"},
	{0x110100, "Performance Counters"},
	{0x111000, "Communication Synchronizer"},
	{0x112000, "Signal Processing Management"},
	{0x118000, "Other Signal Processing Controller"},
	{0x000000, NULL}
};

typedef struct pci_command_t
{
    u8 io_space : 1;
    u8 memory_space : 1;
    u8 bus_master : 1;
    u8 special_cycles : 1;
    u8 memory_write_invalidate_enable : 1;
    u8 vga_palette_snoop : 1;
    u8 parity_error_response : 1;
    u8 RESERVED : 1;
    u8 serr : 1;
    u8 fast_back_to_back : 1;
    u8 interrupt_disable : 1;
    u8 RESERVED2 : 5;
} __attribute__((packed)) pci_command_t;

typedef struct pci_status_t
{
    u8 RESERVED : 3;
    u8 interrupt_status : 1;
    u8 capabilities_list : 1;
    u8 mhz_capable : 1;
    u8 RESERVED2 : 1;
    u8 fast_back_to_back : 1;
    u8 master_data_parity_error : 1;
    u8 devcel : 2;
    u8 signaled_target_abort : 1;
    u8 received_target_abort : 1;
    u8 received_master_abort : 1;
    u8 signaled_system_error : 1;
    u8 detected_parity_error : 1;
} __attribute__((packed)) pci_status_t;

u32 pci_inl(u8 bus, u8 dev, u8 func, u8 addr)
{
    io_out32(PCI_CONF_ADDR, PCI_ADDR(bus, dev, func, addr));
    return io_in32(PCI_CONF_DATA);
}

void pci_outl(u8 bus, u8 dev, u8 func, u8 addr, u32 value)
{
    io_out32(PCI_CONF_ADDR, PCI_ADDR(bus, dev, func, addr));
    io_out32(PCI_CONF_DATA, value);
}

static u32 pci_size(u32 base, u32 mask)
{
    // 去掉必须设置的低位
    u32 size = mask & base;

    // 按位取反再加1得到大小
    size = ~size + 1;

    return size;
}

// 获取某种类型的 Base Address Register
err_t pci_find_bar(pci_device_t *device, pci_bar_t *bar, int type)
{
    for (size_t idx = 0; idx < PCI_BAR_NR; idx++)
    {
        u8 addr = PCI_CONF_BASE_ADDR0 + (idx << 2);
        u32 value = pci_inl(device->bus, device->dev, device->func, addr);
        pci_outl(device->bus, device->dev, device->func, addr, -1);
        u32 len = pci_inl(device->bus, device->dev, device->func, addr);
        pci_outl(device->bus, device->dev, device->func, addr, value);

        if (value == 0)
            continue;

        if (len == 0 || len == -1)
            continue;

        if (value == -1)
            value = 0;

        if ((value & 1) && type == PCI_BAR_TYPE_IO)
        {
            bar->iobase = value & PCI_BAR_IO_MASK;
            bar->size = pci_size(len, PCI_BAR_IO_MASK);
            return EOK;
        }
        if (!(value & 1) && type == PCI_BAR_TYPE_MEM)
        {
            bar->iobase = value & PCI_BAR_MEM_MASK;
            bar->size = pci_size(len, PCI_BAR_MEM_MASK);
            return EOK;
        }
    }
    return -EIO;
}

// 获得 PCI 类型描述
const char *pci_classname(u32 classcode)
{
    for (size_t i = 0; pci_classnames[i].name != NULL; i++)
    {
        if (pci_classnames[i].classcode == classcode)
            return pci_classnames[i].name;
        if (pci_classnames[i].classcode == (classcode & 0xFFFF00))
            return pci_classnames[i].name;
    }
    return "Unknown device";
}

// 检测设备
static void pci_check_device(u8 bus, u8 dev)
{
    u32 value = 0;

    for (u8 func = 0; func < 8; func++)
    {
        value = pci_inl(bus, dev, func, PCI_CONF_VENDOR);
        u16 vendorid = value & 0xffff;
        if (vendorid == 0 || vendorid == 0xFFFF)
            return;

        pci_device_t *device = (pci_device_t *)kmalloc(sizeof(pci_device_t));
        //list_push(&pci_device_list, &device->node);
        list_append(pci_device_list,device);
        
        device->bus = bus;
        device->dev = dev;
        device->func = func;

        device->vendorid = vendorid;
        device->deviceid = value >> 16;

        value = pci_inl(bus, dev, func, PCI_CONF_REVISION);
        device->classcode = value >> 8;
        device->revision = value & 0xff;
    }
}

// 通过供应商/设备号查找设备
pci_device_t *pci_find_device(u16 vendorid, u16 deviceid)
{
    int i;
    for (i=0;i<list_length(pci_device_list);i++)
    {
        pci_device_t *device = list_nth(pci_device_list,i)->data;
        if(device==NULL)continue;
        if (device->vendorid != vendorid)
            continue;
        if (device->deviceid != deviceid)
            continue;
        return device;
    }
    return NULL;
}

// 通过类型查找设备
pci_device_t *pci_find_device_by_class(u32 classcode)
{
    int i;
    for (i=0;i<list_length(pci_device_list);i++)
    {
        pci_device_t *device = list_nth(pci_device_list,i)->data;
        if(device==NULL)continue;
        if (device->classcode == classcode)
            return device;
        if ((device->classcode & PCI_SUBCLASS_MASK) == classcode)
            return device;
    }
    return NULL;
}

// 获得中断 IRQ
u8 pci_interrupt(pci_device_t *device)
{
    u32 data = pci_inl(device->bus, device->dev, device->func, PCI_CONF_INTERRUPT);
    return data & 0xff;
}

// 启用总线主控，用于发起 DMA
void pci_enable_busmastering(pci_device_t *device)
{
    u32 data = pci_inl(device->bus, device->dev, device->func, PCI_CONF_COMMAND);
    data |= PCI_COMMAND_MASTER;
    pci_outl(device->bus, device->dev, device->func, PCI_CONF_COMMAND, data);
}

// PCI 总线枚举
static void pci_enum_device()
{
    for (int bus = 0; bus < 256; bus++)
    {
        for (int dev = 0; dev < 32; dev++)
        {
            pci_check_device(bus, dev);
        }
    }
}

// 初始化 PCI 设备
void pci_init()
{
    pci_device_list=list_alloc(NULL);
    pci_enum_device();
}

// 获得PCI设备总量
int count_pci_device()
{
    return list_length(pci_device_list);
}

void cmd_lspci(console_t *console)
{
    task_t *task=task_now();
    char s[200];
    console_putstr(console,"+----------------------------------------------------------------------------\n");
    if(task->langmode==1 || task->langmode==2)
    {
        console_putstr(console,"|总线 位置  函数  供应商ID  设备ID    类别代码   名称\n");
    }
    else
    {
        console_putstr(console,"|Bus  Slot  Func  VendorID  DeviceID  ClassCode  Name\n");
    }
	
    uint32_t bus,slot,func,vendorid,deviceid,classcode;
    int i;
    for (i=0;i<list_length(pci_device_list);i++)
    {
        pci_device_t *device = list_nth(pci_device_list,i)->data;
        if(device==NULL)continue;

        
        bus=device->bus;
        slot=device->dev;
        func=device->func;
        vendorid=device->vendorid;
        deviceid=device->deviceid;
        classcode=device->classcode;
        sprintf(s,"|%03d  %02d    %02d    0x%04X    0x%04X    0x%06X   %s\n",bus,slot,func,vendorid,deviceid,classcode,pci_classname(classcode));
        console_putstr(console,s);

    }
    console_putstr(console,"+----------------------------------------------------------------------------\n");
}

void klog_lspci()
{
    for (int i=0;i<list_length(pci_device_list);i++)
    {
        pci_device_t *device = list_nth(pci_device_list,i)->data;
        if(device==NULL)continue;
        klogf("PCI: %03d:%02d.%01d: [0x%04x:0x%04x] %s", device->bus, device->dev, device->func, device->vendorid,
              device->deviceid, pci_classname(device->classcode));
    }
}
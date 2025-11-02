/*
com.c
串口驱动程序
Copyright W24 Studio 
*/

#include <com.h>
#include <io.h>
#include <krnlcons.h>
#include <vdisk.h>
#include <stdio.h>

#pragma GCC optimize("00") //硬件处理不开优化

int serial_port_base[4]={0x3f8,0x2f8,0x3e8,0x2e8}; //一般是这样的，具体看BDA（BIOS数据区）
int serial_port_status[4]={0,0,0,0};
vdisk serial_port[4];
int serial_port_drive[4];

static int serial_port_inited=0;

static int init_com_sub(int index)
{
    io_out8(serial_port_base[index] + 1, 0x00); // 禁止COM的中断发生
	io_out8(serial_port_base[index] + 3, 0x80); // 启用DLAB（设置波特率除数）。
	io_out8(serial_port_base[index] + 0, 0x03); // 设置除数为3，(低位) 38400波特
	io_out8(serial_port_base[index] + 1, 0x00); //            (高位)
	io_out8(serial_port_base[index] + 3, 0x03); // 8位，无奇偶性，一个停止位
	io_out8(serial_port_base[index] + 2, 0xC7); // 启用FIFO，有14字节的阈值
	io_out8(serial_port_base[index] + 4, 0x0B); // 启用IRQ，设置RTS/DSR
	io_out8(serial_port_base[index] + 4, 0x1E); // 设置为环回模式，测试串口

    io_out8(serial_port_base[index] + 0, 0xAE); // 测试串口（发送字节0xAE并检查串口是否返回相同的字节）

    /* 检查串口是否有问题（即：与发送的字节不一样） */
	if (io_in8(serial_port_base[index] + 0) != 0xAE) {
		return 1;
	}

	/* 如果串口没有故障，将其设置为正常运行模式 */
	/* (非环回，启用IRQ，启用OUT#1和OUT#2位) */
	io_out8(serial_port_base[index] + 4, 0x0F);
	return 0;
}

static void Read(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{
	for(int i=0;i<4;i++)
	{
		if(serial_port_drive[i]==drive)
		{
			for(int j=0;j<number;j++)
			{
				buffer[j]=read_serial(i);
			}
			break;
		}
	}
}

static void Write(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{
	for(int i=0;i<4;i++)
	{
		if(serial_port_drive[i]==drive)
		{
			serial_putstr_index((char *)buffer,i);
			break;
		}
	}
}

int init_com()
{
	int usable_total=0;
	for(int i=0;i<4;i++)//逐个初始化
	{
		serial_port_status[i]=!init_com_sub(i);
		if(serial_port_status[i]==1)
		{
			usable_total++;
			sprintf(serial_port[i].DriveName,"com%d",i);
			serial_port[i].flag=1;
			serial_port[i].sector_size=1;
			serial_port[i].size=1;
			serial_port[i].Read=Read;
			serial_port[i].Write=Write;
			serial_port_drive[i]=register_vdisk(serial_port[i]);
		}
	}
	klogf("COM:%d available",usable_total);
	serial_port_inited=1;
	return 1;
}

/* 检测串口读是否就绪 */
int serial_received(int index)
{
	return io_in8(serial_port_base[index] + 5) & 1;
}

/* 读串口 */
char read_serial(int index)
{
	if(!serial_port_inited)return -1;
	while (serial_received(index) == 0);
	return io_in8(serial_port_base[index]);
}

/* 检测串口写是否空闲 */
int is_transmit_empty(int index)
{
	return io_in8(serial_port_base[index] + 5) & 0x20;
}

/* 写串口 */
void write_serial(char a)
{
	write_serial_index(a,0);
}

void write_serial_index(char a,int index)
{
	if(!serial_port_inited)return;
	while (is_transmit_empty(index) == 0);
	io_out8(serial_port_base[index], a);
}

/*串口输出字符串*/
void serial_putstr(char *s)
{
	serial_putstr_index(s,0);
}

void serial_putstr_index(char *s,int index)
{
	if(!serial_port_inited)return;
    while(*s)
    {
        write_serial_index(*s,index);
        s++;
    }
}
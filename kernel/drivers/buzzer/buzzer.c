/*
buzzer.c
蜂鸣器驱动程序
Copyright W24 Studio 
*/

#include <buzzer.h>
#include <io.h>

void beep(int freq)
{
    int i;
	if (freq== 0) {
		/* 如果音调（Hz）为0，则关闭板载蜂鸣器 */
		i = io_in8(0x61);					// 读取当前的板载蜂鸣器状态
		io_out8(0x61, i & 0x0d);			// 关闭板载蜂鸣器
	} else {
		/* 计算音调的频率并设置板载蜂鸣器 */
		i = 1193180000 / freq;			// 计算所需的分频值
		io_out8(0x43, 0xb6);				// 发送命令以设置计时器2
		io_out8(0x42, i & 0xff);			// 发送分频值的低字节
		io_out8(0x42, i >> 8);				// 发送分频值的高字节
		i = io_in8(0x61);					// 读取当前板载蜂鸣器状态
		io_out8(0x61, (i | 0x03) & 0x0f);	// 打开板载蜂鸣器
	}
}


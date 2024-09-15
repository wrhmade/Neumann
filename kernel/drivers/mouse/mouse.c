/*
mouse.c
PS/2鼠标驱动程序
Copyright W24 Studio 
*/

#include <mouse.h>
#include <int.h>
#include <io.h>
#include <stdint.h>
#include <graphic.h>
#include <binfo.h>
#include <macro.h>
#include <fifo.h>

#define PORT_KEYDAT		0x0060
#define PORT_KEYCMD		0x0064
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void);

int mouse_data0;

uint32_t mouse_fifobuf[1024];
fifo_t mouse_fifo;

void ps2mouse_handler(registers_t regs)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    mouse_data0=io_in8(PORT_KEYDAT);
    char s[50];
    sprintf(s,"Mouse Input:0x%02X\n",mouse_data0);
    serial_putstr(s);
    fifo_put(&mouse_fifo,mouse_data0);
}

void init_ps2mouse(void)
{
    //注册鼠标中断
    register_interrupt_handler(IRQ12,ps2mouse_handler);


    //开启PS2鼠标

    wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);    

    //初始化鼠标FIFO
    fifo_init(&mouse_fifo,1024,mouse_fifobuf);

}

void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}


int mouse_decode(mdec_t *mdec, uint8_t dat)
{
    if (mdec->phase == 0) {
        /* 在phase=0时,表处于等待鼠标回复0xfa
         * 阶段,等到则置接收鼠标第一个字节数据阶段。*/
        if (dat == 0xfa) {
            mdec->phase = 1;
        }
        return 0;
    }
    if (mdec->phase == 1) {
        /* dat鼠标第1字节数据 */
        if ((dat & 0xc8) == 0x08) {
            /* 据作者观察(当然此文也观察过), 鼠标第1字
             * 节数据bit[3]=1,bit[7..6]=00,若鼠标第1字
             * 节数据不满足以上状态则表明 鼠标数据传递
             * 可能有误,所以在此丢弃并继续等待并解析第
             * 一字节数据。
             * 
             * bit[2..0]置位时分别代表鼠标滑轮点击, 鼠
             * 标右击和鼠标左击。bit[5..4]分别跟鼠标上
             * 下和左右移动的方向, 值为0时表示往上或右
             * 移动,值为1时表示往下或左移动。
             * 
             * 接收到第一个字节后置phase=2表示接下来接
             * 收鼠标第二字节数据。*/
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if (mdec->phase == 2) {
        /* 接收鼠标第二字节数据(左右移动)并置
         * phase=3以接收鼠标第三字节数据。*/
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if (mdec->phase == 3) {
        /* 鼠标第3字节数据接收完毕,置phase=1
         * 表继续接收下一组鼠标数据。*/
        mdec->buf[2] = dat;
        mdec->phase = 1;
    
        /* 一组鼠标数据接收完毕,开始解析。*/

        /* btn,点击事件;x,左右移动位移量;
         * y,上下移动位移量。*/
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];
        
        /* 若鼠标第1字节bit[5..4]皆为1,则表示鼠标分别
         * 在往下或左方向移动, 鼠标将这两个方向分别视
         * 为y和x的负方向。与此对应, 此时鼠标中断传送
         * 上来的y和x是一个负数( 的低8位)。在32位补码
         * 表示数中, 将y和x的高24位扩展为1时, 就得到y
         * 和x的负数值, 从而获得了鼠标位移的真实值。*/
        if ((mdec->buf[0] & 0x10) != 0) {
            mdec->x |= 0xffffff00;
        }
        if ((mdec->buf[0] & 0x20) != 0) {
            mdec->y |= 0xffffff00;
        }
        
        /* 屏幕显示画面时原点在左上角,而鼠
         * 标移动位移量的原点在屏幕左下角。
         * 所以鼠标在y方向的位移值方向跟屏
         * 幕实际坐标方向相反,所以此处进行
         * 符号取反。*/
        mdec->y = - mdec->y;
        return 1;
    }
    return -1;
}
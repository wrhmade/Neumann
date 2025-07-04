/*
lpt.c
打印机接口
Copyright W24 Studio 
*/

#include <lpt.h>
#include <io.h>
#include <timer.h>

#pragma GCC optimize("00") //硬件处理不开优化

void wait_lpt_ready()
{
    while(!io_in8(LPT1_PORT_STATUS)&0x80)
    {
        sleep(10);
    };
}

void lpt_put(unsigned char c)
{
    unsigned char lControl;

    wait_lpt_ready();//等待并口就绪
    io_out8(LPT1_PORT_DATA,c);//向并口输出一个字符
    wait_lpt_ready();

    //现在脉冲连接线，告诉打印机读取数据
    lControl=io_in8(LPT1_PORT_CONTROL);
    io_out8(LPT1_PORT_CONTROL,lControl|1);
    sleep(10);
    io_out8(LPT1_PORT_CONTROL,lControl);

    wait_lpt_ready();
}
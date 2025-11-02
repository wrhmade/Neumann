/*
lpt.c
打印机接口
Copyright W24 Studio 
*/

#include <lpt.h>
#include <io.h>
#include <timer.h>
#include <vdisk.h>
#include <krnlcons.h>
#include <stdio.h>

#pragma GCC optimize("00") //硬件处理不开优化

int lpt_base[3]={0x378,0x278,0x3bc}; //一般是这样的，具体看BDA（BIOS数据区），前两个为标准ISA线上的，第三个一般为拓展

int lpt_status[3]={0,0,0};
vdisk lpt_vd[3];
int lpt_drive[3];

static void Read(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{
    for(int i=0;i<3;i++)
    {
        if(lpt_drive[i]==drive)
        {
            for(int j=0;j<number;j++)
            {
                buffer[i]=lpt_read(i);
            }
        }
    }
}

static void Write(int drive, uint8_t *buffer, uint32_t number, uint32_t lba)
{
    for(int i=0;i<3;i++)
    {
        if(lpt_drive[i]==drive)
        {
            for(int j=0;j<number;j++)
            {
                lpt_put(buffer[j],i);
            }
        }
    }
}


static int check_lpt(int n)
{
    unsigned char test_word[]={0x55, 0xAA, 0x00, 0xFF};
    for(int i=0;i<sizeof(test_word)/sizeof(char);i++)
    {
        io_out8(LPT_DATA(n),test_word[i]);
        unsigned char data=io_in8(LPT_DATA(n));
        if(data!=test_word[i])
        {
            return -1;
        }
    }
    return 0;
}

void init_lpt()
{
    int usable_total=0;
    for(int i=0;i<3;i++)
    {
        if(check_lpt(i)==-1)continue;
        sprintf(lpt_vd[i].DriveName,"lpt%d",i);
        lpt_vd[i].flag=1;
        lpt_vd[i].sector_size=1;
        lpt_vd[i].size=1;
        lpt_vd[i].Read=Read;
        lpt_vd[i].Write=Write;
        lpt_drive[i]=register_vdisk(lpt_vd[i]);
        lpt_status[i]=1;
        usable_total++;
    }
    klogf("LPT:%d available",usable_total);
}

void wait_lpt_ready(int i)
{
    while(!(io_in8(LPT_STATUS(i))&0x80))
    {
        sleep(10);
    };
}

void lpt_put(unsigned char c,int i)
{
    if(!lpt_status[i])return;
    unsigned char lControl;

    wait_lpt_ready(i);//等待并口就绪
    io_out8(LPT_DATA(i),c);//向并口输出一个字符
    wait_lpt_ready(i);

    //现在脉冲连接线，告诉打印机读取数据
    lControl=io_in8(LPT_CONTROL(i));
    io_out8(LPT_CONTROL(i),lControl|1);
    sleep(10);
    io_out8(LPT_CONTROL(i),lControl);

    wait_lpt_ready(i);
}

int lpt_read(int i)
{
    if(!lpt_status[i])return -1;
    wait_lpt_ready(i);
    int a=io_in8(LPT_DATA(i));
    wait_lpt_ready(i);
    return a;
}
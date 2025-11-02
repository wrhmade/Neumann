/*
sb16.c
SB16声卡驱动
Copyright W24 Studio 
*/

#include <audio.h>
#include <sb16.h>
#include <io.h>
#include <timer.h>
#include <krnlcons.h>
#include <stdio.h>
#include <string.h>
#include <dma.h>
#include <int.h>
#include <mm.h>
#include <string.h>
#include <acpi.h>
#include <int.h>

static volatile int sig = 0;
uint32_t buf_phy = 0x2000; // 一个临时的数据区，必须在8位范围（ISA规定）

/* SB16中断 */
static void sb16_handler(registers_t *regs)
{
    io_in8(SB_INTR16);
    sig = 1;
    send_eoi();
}

/* 向SB16发送数据 */
static void sb_out(uint8_t value)
{
    while (io_in8(SB_WRITE) & 0x80);
    io_out8(SB_WRITE, value);
}

/* 初始化SB16驱动 */
int sb16_init(void)
{
    io_out8(SB_RESET, 1);
    sleep(50);
    io_out8(SB_RESET, 0);

    if (io_in8(SB_STATE) == 0x80) {
        io_out8(SB_MIXER, 0x80);
        io_out8(SB_MIXER_DATA, 0x02);
        sb_out(DSP_CMD_SPEAKER_ON);

        register_interrupt_handler(IRQ5, sb16_handler);

        audio_device_t sb16_device;
        sb16_device.flag=1;
        sb16_device.name=strdup("sb16");
        sb16_device.setvolume=sb16_set_volume;
        sb16_device.play=sb16_play;
        sb16_device.stop=sb16_stop;
        sb16_device.open_speaker=sb16_open_speaker;
        sb16_device.close_speaker=sb16_close_speaker;
        sb16_device.wait=sb16_wait;
        audio_device_reg(sb16_device);
        return 0;
    } else {
        return 1;
    }
}

/* SB16设置音量 */
void sb16_set_volume(uint8_t left, uint8_t right)
{
    if (left > 15) left = 15;
    if (right > 15) right = 15;

    io_out8(SB_MIXER, 0x22);
    io_out8(SB_MIXER_DATA, (left << 4) | (right & 0x0F));
}

/* 设置默认采样率 */
void sb16_set_sample_rate(uint16_t rate)
{
    sb_out(0x41);
    sb_out((rate >> 8) & 0xff);
    sb_out(rate & 0xff);
}

/* 向SB16发送数据包 */
void sb16_send_data(const uint8_t *data, unsigned int size,pcm_type_t pcm_type,int channels)
{
    uint16_t count = (size / 2 - 1);

    if (size > DMA_BUF_SIZE) size = DMA_BUF_SIZE;
    for (unsigned int i = 0; i < size; i++) ((uint8_t *)buf_phy)[i] = data[i];

    switch(pcm_type)
    {
        case PCM_UNSIGNED_8BIT:dma_send(1, (uint32_t *)buf_phy, size);break;
        case PCM_SIGNED_16BIT:dma_send(5, (uint32_t *)buf_phy, size);break;
        default:break;
    }

    switch(pcm_type)
    {
        case PCM_UNSIGNED_8BIT:sb_out(0xC0);break;
        case PCM_SIGNED_16BIT:sb_out(0xB0);break;
        default:break;
    }

    sb_out(0x10);
    sb_out(count & 0xff);
    sb_out((count >> 8) & 0xff);
}

/* 使用SB16播放PCM数据 */
void sb16_play(uint8_t *pcmdata,int len,int sample_rate,pcm_type_t pcm_type,int channels)
{
    sb16_set_sample_rate(sample_rate);

    unsigned int offset = 0;
    while (offset < len) {
        unsigned int chunk = len - offset;
        if (chunk > DMA_BUF_SIZE) chunk = DMA_BUF_SIZE;
        sb16_send_data(pcmdata + offset, chunk, pcm_type,channels);
        while (!sig);
        sig     = 0;
        offset += chunk;
    }
}

void sb16_stop()
{
    sb_out(0xD0);
    io_out8(0x0A, 0x05);
    io_out8(0x0C, 0x00);
    sb16_wait();
}

void sb16_wait()
{
    while(!(io_in8(SB_STATE)&0x80));
}

void sb16_open_speaker()
{
    sb_out(0xD1);
}

void sb16_close_speaker()
{
    sb_out(0xD3);
}
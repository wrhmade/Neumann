/*
sb16.h
SB16声卡驱动头文件
Copyright W24 Studio 
*/

#ifndef SB16_H
#define SB16_H

#include <stdint.h>
#include <audio.h>

#define SB_MIXER      0x224 // DSP 混音器端口
#define SB_MIXER_DATA 0x225 // DSP 混音器数据端口
#define SB_RESET      0x226 // DSP 复位
#define SB_READ       0x22A // DSP 读取
#define SB_WRITE      0x22C // DSP 写入
#define SB_STATE      0x22E // DSP 读取状态
#define SB_INTR16     0x22F // DSP 16 位中断确认

#define DSP_CMD_SET_TIME_CONST  0x40 // 设置时间常数
#define DSP_CMD_SET_SAMPLE_RATE 0x41 // 设置输出采样率
#define DSP_CMD_SPEAKER_ON      0xD1 // 打开扬声器
#define DSP_CMD_SPEAKER_OFF     0xD3 // 关闭扬声器
#define DSP_CMD_STOP_8BIT       0xD0 // 停止播放 8 位通道
#define DSP_CMD_RESUME_8BIT     0xD4 // 恢复播放 8 位通道
#define DSP_CMD_STOP_16BIT      0xD5 // 停止播放 16 位通道
#define DSP_CMD_RESUME_16BIT    0xD6 // 恢复播放 16 位通道
#define DSP_CMD_GET_VERSION     0xE1 // 获取 DSP 版本

#define MIXER_REG_MASTER_VOL 0x22 // 主音量
#define MIXER_REG_IRQ        0x80 // 设置 IRQ

#define DMA_BUF_SIZE 0x8000 // DMA 缓冲区大小

int sb16_init(void);
void sb16_set_volume(uint8_t left, uint8_t right);
void sb16_set_sample_rate(uint16_t rate);
void sb16_send_data(const uint8_t *data, unsigned int size,pcm_type_t pcm_type,int channels);
void sb16_play(uint8_t *pcmdata,int len,int sample_rate,pcm_type_t pcm_type,int channels);
void sb16_open_speaker();
void sb16_close_speaker();
void sb16_stop();
void sb16_wait();
#endif
/*
audio.h
声卡驱动头文件
Copyright W24 Studio 
*/

#ifndef AUDIO_H
#define AUDIO_H
#include <stdint.h>

#define AUDIO_DEVICE_MAX 32

typedef enum PCM_TYPE
{
    PCM_UNSIGNED_8BIT,
    PCM_SIGNED_16BIT,
    PCM_SIGNED_24BIT,
    PCM_SIGNED_32BIT,
}pcm_type_t;

typedef void (*audio_device_setvolume_t)(uint8_t,uint8_t);
typedef void (*audio_device_play_t)(uint8_t  *,int,int,pcm_type_t,int);
typedef void (*audio_device_stop_t)();
typedef void (*audio_device_wait_t)();
typedef void (*audio_device_open_speaker_t)();
typedef void (*audio_device_close_speaker_t)();

typedef struct AUDIO_DEVICE
{
    char *name;
    int flag;
    audio_device_setvolume_t setvolume;
    audio_device_play_t play;
    audio_device_stop_t stop;
    audio_device_wait_t wait;
    audio_device_open_speaker_t open_speaker;
    audio_device_close_speaker_t close_speaker;
}audio_device_t;

void audio_device_init();
int audio_device_reg(audio_device_t device);
int audio_device_get_index_by_name(char *name);
int audio_device_setvolume(int index,char left,char right);
int audio_device_play(int index,uint8_t *pcmdata,int len,int sample_rate,pcm_type_t pcm_type,int channels);
int audio_device_stop(int index);
int audio_device_wait(int index);
int audio_device_openspeaker(int index);
int audio_device_closespeaker(int index);
#endif
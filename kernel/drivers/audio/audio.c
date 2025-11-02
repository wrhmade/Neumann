/*
audio.c
声卡驱动
Copyright W24 Studio 
*/

#include <string.h>
#include <audio.h>
#include <stdint.h>

audio_device_t audio_devices[AUDIO_DEVICE_MAX];

void audio_device_init()
{
    for(int i=0;i<AUDIO_DEVICE_MAX;i++)
    {
        audio_devices[i].flag=0;
    }
}

int audio_device_reg(audio_device_t device)
{
    for(int i=0;i<AUDIO_DEVICE_MAX;i++)
    {
        if(audio_devices[i].flag==0)
        {
            audio_devices[i]=device;
            return i;
        }
    }
    return -1;
}

int audio_device_get_index_by_name(char *name)
{
    for(int i=0;i<AUDIO_DEVICE_MAX;i++)
    {
        if(audio_devices[i].flag)
        {
            if(strcmp(audio_devices[i].name,name)==0)
            {
                return i;
            }
        }
    }
    return -1;
}

int audio_device_setvolume(int index,char left,char right)
{
    if(audio_devices[index].flag==0)
    {
        return -1;
    }
    if(!audio_devices[index].setvolume)
    {
        return -1;
    }
    
    audio_devices[index].setvolume(left,right);
    return 0;
}

int audio_device_play(int index,uint8_t *pcmdata,int len,int sample_rate,pcm_type_t pcm_type,int channels)
{
    if(audio_devices[index].flag==0)
    {
        return -1;
    }
    if(!audio_devices[index].play)
    {
        return -1;
    }
    
    audio_devices[index].play(pcmdata,len,sample_rate,pcm_type,channels);
    return 0;
}

int audio_device_stop(int index)
{
    if(audio_devices[index].flag==0)
    {
        return -1;
    }
    if(!audio_devices[index].stop)
    {
        return -1;
    }
    
    audio_devices[index].stop();
    return 0;
}

int audio_device_wait(int index)
{
    if(audio_devices[index].flag==0)
    {
        return -1;
    }
    if(!audio_devices[index].wait)
    {
        return -1;
    }
    
    audio_devices[index].wait();
    return 0;
}

int audio_device_openspeaker(int index)
{
    if(audio_devices[index].flag==0)
    {
        return -1;
    }
    if(!audio_devices[index].open_speaker)
    {
        return -1;
    }
    
    audio_devices[index].open_speaker();
    return 0;
}

int audio_device_closespeaker(int index)
{
    if(audio_devices[index].flag==0)
    {
        return -1;
    }
    if(!audio_devices[index].close_speaker)
    {
        return -1;
    }
    
    audio_devices[index].close_speaker();
    return 0;
}
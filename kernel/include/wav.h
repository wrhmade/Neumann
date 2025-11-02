/*
wav.h
WAV音频文件解析器头文件
Copyright W24 Studio 
*/

#ifndef WAV_H
#define WAV_H
#include <stdint.h>

// WAV文件头结构体
#pragma pack(push, 1)
typedef struct {
    char chunkID[4];        // "RIFF"
    uint32_t chunkSize;     // 文件总大小-8
    char format[4];         // "WAVE"
    
    char subchunk1ID[4];    // "fmt "
    uint32_t subchunk1Size; // PCM为16
    uint16_t audioFormat;   // PCM为1
    uint16_t numChannels;   // 声道数
    uint32_t sampleRate;    // 采样率
    uint32_t byteRate;      // 每秒字节数
    uint16_t blockAlign;    // 每个样本的字节数
    uint16_t bitsPerSample; // 每样本位数
} WAVHeader;
#pragma pack(pop)

// LIST块中的信息标签结构
typedef struct {
    char tag[4];           // 标签名，如 "IART", "ICMT" 等
    uint32_t size;         // 数据大小
    // 数据内容紧随其后（可变长度）
} InfoTag;

// 提取的元数据结构
typedef struct {
    char* artist;          // 艺术家
    char* comment;         // 注释
    char* copyright;       // 版权信息
    char* creationDate;    // 创建日期
    char* engineer;        // 工程师
    char* software;        // 软件
    char* title;           // 标题
    char* subject;         // 主题
} WAVMetadata;

void initMetadata(WAVMetadata* metadata);
void freeMetadata(WAVMetadata* metadata);
int parseListChunk(unsigned char* listData, uint32_t listSize, WAVMetadata* metadata);
int extractPCMFromBufferEx(unsigned char* buffer, uint32_t bufferSize, unsigned char** pcmData, uint32_t* pcmSize,WAVHeader* header, WAVMetadata* metadata);
#endif
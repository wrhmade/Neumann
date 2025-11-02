/*
wav.c
WAV音频文件解析器
Copyright W24 Studio 
*/

#include <wav.h>
#include <stdint.h>
#include <string.h>
#include <mm.h>

// 初始化元数据结构
void initMetadata(WAVMetadata* metadata) {
    metadata->artist = NULL;
    metadata->comment = NULL;
    metadata->copyright = NULL;
    metadata->creationDate = NULL;
    metadata->engineer = NULL;
    metadata->software = NULL;
    metadata->title = NULL;
    metadata->subject = NULL;
}

// 释放元数据内存
void freeMetadata(WAVMetadata* metadata) {
    if (metadata->artist) kfree(metadata->artist);
    if (metadata->comment) kfree(metadata->comment);
    if (metadata->copyright) kfree(metadata->copyright);
    if (metadata->creationDate) kfree(metadata->creationDate);
    if (metadata->engineer) kfree(metadata->engineer);
    if (metadata->software) kfree(metadata->software);
    if (metadata->title) kfree(metadata->title);
    if (metadata->subject) kfree(metadata->subject);
}

// 解析LIST块中的元数据
int parseListChunk(unsigned char* listData, uint32_t listSize, WAVMetadata* metadata) {
    if (listSize < 4) return -1;
    
    // 检查LIST块类型
    char listType[5] = {0};
    memcpy(listType, listData, 4);
    
    if (memcmp(listType, "INFO", 4) != 0) {
        return -1;
    }
    
    uint32_t offset = 4;  // 跳过"INFO"
    
    while (offset < listSize - 8) {
        InfoTag tag;
        memcpy(tag.tag, listData + offset, 4);
        tag.size = *(uint32_t*)(listData + offset + 4);
        
        // 确保大小有效
        if (tag.size > listSize - offset - 8 || tag.size == 0) {
            break;
        }
        
        // 分配内存并复制数据（去掉可能的空终止符）
        char* data = (char*)kmalloc(tag.size);
        if (data) {
            memcpy(data, listData + offset + 8, tag.size);
            
            // 根据标签类型存储数据
            if (memcmp(tag.tag, "IART", 4) == 0) {
                metadata->artist = data;
            } else if (memcmp(tag.tag, "ICMT", 4) == 0) {
                metadata->comment = data;
            } else if (memcmp(tag.tag, "ICOP", 4) == 0) {
                metadata->copyright = data;
            } else if (memcmp(tag.tag, "ICRD", 4) == 0) {
                metadata->creationDate = data;
            } else if (memcmp(tag.tag, "IENG", 4) == 0) {
                metadata->engineer = data;
            } else if (memcmp(tag.tag, "ISFT", 4) == 0) {
                metadata->software = data;
            } else if (memcmp(tag.tag, "INAM", 4) == 0) {
                metadata->title = data;
            } else if (memcmp(tag.tag, "ISBJ", 4) == 0) {
                metadata->subject = data;
            } else {
                kfree(data);  // 不认识的标签，释放内存
            }
        }
        
        offset += 8 + tag.size;
        // 对齐到偶数字节边界
        if (tag.size % 2 != 0) offset++;
    }
    
    return 0;
}

// 增强版的PCM数据提取函数
int extractPCMFromBufferEx(unsigned char* buffer, uint32_t bufferSize, 
                          unsigned char** pcmData, uint32_t* pcmSize,
                          WAVHeader* header, WAVMetadata* metadata) {
    
    if (bufferSize < sizeof(WAVHeader)) {
        return -1;
    }
    
    // 解析基础WAV头（不包含数据块信息）
    memcpy(header, buffer, 36);  // 只复制到bitsPerSample为止
    
    // 验证文件格式
    if (memcmp(header->chunkID, "RIFF", 4) != 0 ||
        memcmp(header->format, "WAVE", 4) != 0) {
        return -1;
    }
    
    if (header->audioFormat != 1) {
        return -1;
    }
    
    // 遍历所有块
    uint32_t offset = 12;  // 跳过RIFF头
    
    while (offset < bufferSize - 8) {
        char chunkID[5] = {0};
        memcpy(chunkID, buffer + offset, 4);
        uint32_t chunkSize = *(uint32_t*)(buffer + offset + 4);
        
        if (memcmp(chunkID, "fmt ", 4) == 0) {
            // fmt块已经在初始头中解析过了，跳过
            offset += 8 + chunkSize;
        } else if (memcmp(chunkID, "data", 4) == 0) {
            // 找到数据块
            *pcmData = (unsigned char*)kmalloc(chunkSize);
            if (*pcmData == NULL) {
                return -1;
            }
            
            memcpy(*pcmData, buffer + offset + 8, chunkSize);
            *pcmSize = chunkSize;
            offset += 8 + chunkSize;
            break;  // 找到数据块后可以退出，或者继续处理后面的块
        } else if (memcmp(chunkID, "LIST", 4) == 0) {
            // 处理LIST块（元数据）       
            if (metadata && chunkSize > 0) {
                parseListChunk(buffer + offset + 8, chunkSize, metadata);
            }
            
            offset += 8 + chunkSize;
        } else {
            // 其他未知块，跳过
            offset += 8 + chunkSize;
        }
        
        // 对齐到偶数字节边界
        if (chunkSize % 2 != 0) offset++;
        
        if (offset >= bufferSize) break;
    }
    
    if (*pcmData == NULL) {
        return -1;
    }
    
    return 0;
}
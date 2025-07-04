// Author: foolish-shabby <2276316223@qq.com>
// License: WTFPL
// This software is a part of free toolpack 'myfattools', which is aimed to operate FAT images in CLI.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define malloc(size) calloc(size, 1)

unsigned char default_boot_code[] = {
    0x8c, 0xc8, 0x8e, 0xd8, 0x8e, 0xc0, 0xb8, 0x00, 0x06, 0xbb, 0x00, 0x07, 0xb9, 0x00, 0x00, 0xba,
    0x4f, 0x18, 0xcd, 0x10, 0xb6, 0x00, 0xe8, 0x02, 0x00, 0xeb, 0xfe, 0xb8, 0x6c, 0x7c, 0x89, 0xc5,
    0xb9, 0x2a, 0x00, 0xb8, 0x01, 0x13, 0xbb, 0x07, 0x00, 0xb2, 0x00, 0xcd, 0x10, 0xc3, 0x46, 0x41,
    0x54, 0x41, 0x4c, 0x3a, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x61, 0x20, 0x62, 0x6f, 0x6f, 0x74, 0x61,
    0x62, 0x6c, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6b, 0x2e, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d,
    0x20, 0x68, 0x61, 0x6c, 0x74, 0x65, 0x64, 0x2e, 0x00, 0x00
};

typedef struct FAT_BPB_HEADER {
    unsigned char BS_jmpBoot[3];
    unsigned char BS_OEMName[8];
    unsigned short BPB_BytsPerSec;
    unsigned char BPB_SecPerClust;
    unsigned short BPB_RsvdSecCnt;
    unsigned char BPB_NumFATs;
    unsigned short BPB_RootEntCnt;
    unsigned short BPB_TotSec16;
    unsigned char BPB_Media;
    unsigned short BPB_FATSz16;
    unsigned short BPB_SecPerTrk;
    unsigned short BPB_NumHeads;
    unsigned int BPB_HiddSec;
    unsigned int BPB_TotSec32;
    unsigned char BS_DrvNum;
    unsigned char BS_Reserved1;
    unsigned char BS_BootSig;
    unsigned int BS_VolID;
    unsigned char BS_VolLab[11];
    unsigned char BS_FileSysType[8];
    unsigned char BS_BootCode[448];
    unsigned short BS_BootEndSig;
} __attribute__((packed)) bpb_hdr_t;

void print_usage(char *name)
{
    puts("ftimage 0.0.1 by foolish-shabby");
    printf("Usage: %s <filename> <-size size> [-bs boot_sector] [-h]\n", name);
    puts("Valid Options:");
    puts("    -size    Specify the size of the image, in megabytes");
    puts("    -bs      Specify boot sector");
    puts("    -h       Show this message");
}

char *filename, *boot_sector;
int size;

int ftimage()
{
    FILE *fp = fopen(filename, "wb");
    for (int i = 0; i < size; i++) fputc('\0', fp);
    fclose(fp);
    fp = fopen(filename, "rb+");
    if (!boot_sector) {
        bpb_hdr_t hdr;
        fseek(fp, 0, SEEK_END);
        long int fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        long sectors = fsize / 512;
        hdr.BS_jmpBoot[0] = 0xeb;
        hdr.BS_jmpBoot[1] = 0x3c;
        hdr.BS_jmpBoot[2] = 0x90;
        strcpy(hdr.BS_OEMName, "FATTOOLS");
        hdr.BPB_BytsPerSec = 512;
        hdr.BPB_SecPerClust = 1;
        hdr.BPB_RsvdSecCnt = 1;
        hdr.BPB_NumFATs = 2;
        hdr.BPB_RootEntCnt = 512;
        if (sectors < (1 << 16) - 1) {
            hdr.BPB_TotSec16 = sectors;
            hdr.BPB_TotSec32 = 0;
        } else {
            hdr.BPB_TotSec16 = 0;
            hdr.BPB_TotSec32 = sectors;
        }
        hdr.BPB_Media = 0xf8;
        hdr.BPB_FATSz16 = 32;
        hdr.BPB_SecPerTrk = 63;
        hdr.BPB_NumHeads = 16;
        hdr.BPB_HiddSec = 0;
        hdr.BS_DrvNum = 0x80;
        hdr.BS_Reserved1 = 0;
        hdr.BS_BootSig = 0x29;
        hdr.BS_VolID = 0;
        strcpy(hdr.BS_VolLab, "FOOLISHABBY");
        strcpy(hdr.BS_FileSysType, "FAT16   ");
        memset(hdr.BS_BootCode, 0, 448);
        memcpy(hdr.BS_BootCode, default_boot_code, sizeof(default_boot_code));
        hdr.BS_BootEndSig = 0xaa55;
        fwrite(&hdr, 512, 1, fp);
        fflush(fp);
    } else {
        FILE *bs = fopen(boot_sector, "rb");
        char *boot = (char *) malloc(512);
        fread(boot, 512, 1, bs);
        fclose(bs);
        fwrite(boot, 512, 1, fp);
        fflush(fp);
    }
    fseek(fp, 512, SEEK_SET);
    char initial_fat[4] = {0};
    initial_fat[1] = initial_fat[2] = 0xff;
    initial_fat[0] = initial_fat[3] = 0xff;
    fwrite(initial_fat, 4, 1, fp);
    fflush(fp);
    fseek(fp, 33 * 512, SEEK_SET);
    fwrite(initial_fat, 4, 1, fp);
    fflush(fp);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            print_usage(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-bs")) {
            i++;
            boot_sector = malloc(strlen(argv[i]) + 5);
            strcpy(boot_sector, argv[i]);
        } else if (!strcmp(argv[i], "-size")) {
            i++;
            int len = strlen(argv[i]); 
            // 不想破坏argv，所以采用这种遍历方法
            for (int j = 0; j < len; j++) {
                if (argv[i][j] < '0' || argv[i][j] > '9') { // 如果不是数字（负号也不行）
                    printf("Error: Expected positive number after -c, '%s' occurred", argv[i]); // 报错退出
                    return 1;
                }
                size = size * 10 + argv[i][j] - '0'; // 先乘10把前几位平移留出最后一位，然后把当前字符-'0'转化成数字加在末位
            }
            size *= 1048576;
        } else {
            if (filename) { // 在此之前已经有文件名，报错
                printf("Error: multiple filenames (the first one is: '%s')\n", filename);
                return 1;
            }
            filename = malloc(strlen(argv[i]) + 5);
            if (!filename) { // 分配内存失败，报错
                printf("Error: no memory for filename\n");
                return 1;
            }
            strcpy(filename, argv[i]); // 复制一个，不用argv了
        }
    }
    if (!filename) { // 参数中没有文件名，报错
        printf("Error: filename required\n");
        return 1;
    }
    int ret = ftimage();
    free(filename);
    return ret;
}
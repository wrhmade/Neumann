#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef enum IMAGE_TYPE {
    FD, HD
} im_type_t;

typedef enum FILE_SYSTEM_TYPE {
    FAT12, FAT16
} fs_type_t;

void print_usage(char *name)
{
    puts("ftformat 0.0.1 by foolish-shabby");
    printf("Usage: %s <filename> [-t hd|fd] [-f fat12|fat16] [-h]\n", name);
    puts("Valid Options:");
    puts("    -t       Specify the type of image (fd or hd), 'fd' in default");
    puts("    -f       Specify the type of file system (fat12 or fat16), 'fat12' in default");
    puts("    -h       Show this message");
}

char *filename;
im_type_t im_type = FD;
fs_type_t fs_type = FAT12;

int ftformat()
{
    bpb_hdr_t hdr;
    FILE *fp = fopen(filename, "rb+");
    fseek(fp, 0, SEEK_END);
    long int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    long sectors = fsize / 512;
    hdr.BS_jmpBoot[0] = 0xeb;
    hdr.BS_jmpBoot[1] = 0x3c;
    hdr.BS_jmpBoot[2] = 0x90;
    strcpy(hdr.BS_OEMName, "FTFORMAT");
    hdr.BPB_BytsPerSec = 512;
    hdr.BPB_SecPerClust = 1;
    hdr.BPB_RsvdSecCnt = 1;
    hdr.BPB_NumFATs = 2;
    hdr.BPB_RootEntCnt = fs_type == FAT16 ? 512 : 224;
    if (sectors < (1 << 16) - 1) {
        hdr.BPB_TotSec16 = sectors;
        hdr.BPB_TotSec32 = 0;
    } else {
        hdr.BPB_TotSec16 = 0;
        hdr.BPB_TotSec32 = sectors;
    }
    hdr.BPB_Media = im_type == HD ? 0xf8 : 0xf0;
    hdr.BPB_FATSz16 = fs_type == FAT16 ? 32 : 9;
    hdr.BPB_SecPerTrk = im_type == HD ? 63 : 18;
    hdr.BPB_NumHeads = im_type == HD ? 16 : 2;
    hdr.BPB_HiddSec = 0;
    hdr.BS_DrvNum = im_type == HD ? 0x80 : 0;
    hdr.BS_Reserved1 = 0;
    hdr.BS_BootSig = 0x29;
    hdr.BS_VolID = 0;
    strcpy(hdr.BS_VolLab, "FOOLISHABBY");
    strcpy(hdr.BS_FileSysType, "FAT12   ");
    if (fs_type == FAT16) hdr.BS_FileSysType[4] = '6';
    memset(hdr.BS_BootCode, 0, 448);
    memcpy(hdr.BS_BootCode, default_boot_code, sizeof(default_boot_code));
    hdr.BS_BootEndSig = 0xaa55;
    fwrite(&hdr, 512, 1, fp);
    fflush(fp);

    fseek(fp, 512, SEEK_SET);
    char initial_fat[4] = {0};
    initial_fat[1] = initial_fat[2] = 0xff;
    if (fs_type == FAT12) initial_fat[0] = 0xf0, initial_fat[3] = 0;
    else initial_fat[0] = initial_fat[3] = 0xff;
    fwrite(initial_fat, 4, 1, fp);
    fflush(fp);
    if (fs_type == FAT12) fseek(fp, 10 * 512, SEEK_SET);
    else fseek(fp, 33 * 512, SEEK_SET);
    fwrite(initial_fat, 4, 1, fp);
    fflush(fp);
    return 0;
}

int ftformat_main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            print_usage(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-t")) {
            i++;
            if (!strcmp(argv[i], "hd")) {
                im_type = HD;
            } else if (!strcmp(argv[i], "fd")) {
                im_type = FD;
            } else {
                printf("Invalid image type: 'fd' or 'hd' expected, '%s' occurred", argv[i]);
                return 1;
            }
        } else if (!strcmp(argv[i], "-f")) {
            i++;
            if (!strcmp(argv[i], "fat12")) {
                fs_type = FAT12;
            } else if (!strcmp(argv[i], "fat16")) {
                fs_type = FAT16;
            } else {
                printf("Invalid file system type: 'fat12' or 'fat16' expected, '%s' occurred", argv[i]);
                return 1;
            }
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
    int ret = ftformat();
    free(filename);
    return ret;
}

#ifndef STANDALONE
int main(int argc, char **argv)
{
    return ftformat_main(argc, argv);
}
#endif
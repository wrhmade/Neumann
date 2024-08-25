#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct FILEINFO {
    char name[8], ext[3];
    unsigned char type, reserved[10];
    unsigned short time, date, clustno;
    unsigned int size;
} __attribute__((packed)) fileinfo_t;

typedef struct {
    char name[255]; // prepare for LFN
    unsigned int clustno;
    unsigned int size;
} file_t;

typedef enum FILE_SYSTEM_TYPE {
    FAT12, FAT16
} fs_type_t;

char *filename;
char *imgname;
fs_type_t fs_type;

void print_usage(char *name)
{
    puts("ftrm INDEV by foolish-shabby");
    printf("Usage: %s <filename> <-img imgname> [-h]\n", name);
    puts("Valid options:");
    puts("    -img     Specify the image to operate");
    puts("    -h       Show this message");
}

unsigned short get_next_clustno(FILE *fp, unsigned int first_clustno)
{
    long int pos = ftell(fp);
    int fat_start = 1;
    unsigned char *fat = (unsigned char *) malloc(2 * 512);
    unsigned int fat_offset = fs_type == FAT12 ? first_clustno + (first_clustno / 2) : first_clustno * 2;
    unsigned int fat_sect = fat_start + (fat_offset / 512);
    unsigned int sect_offset = fat_offset % 512;
    fseek(fp, fat_sect * 512, SEEK_SET);
    fread(fat, 2, 512, fp);
    unsigned short table_val = *(unsigned short *) &fat[sect_offset];
    if (fs_type == FAT12) table_val = (first_clustno & 1) ? table_val >> 4 : table_val & 0xfff;
    fseek(fp, pos, SEEK_SET);
    free(fat);
    return table_val;
}

void set_nth_clustno(FILE *fp, unsigned int first_clustno, unsigned short val)
{
    long int pos = ftell(fp);
    int fat_start = 1;
    int second_fat_start = fs_type == FAT12 ? 10 : 33;
    unsigned char *fat = (unsigned char *) malloc(2 * 512);
    unsigned int fat_offset = fs_type == FAT12 ? first_clustno + (first_clustno / 2) : first_clustno * 2;
    unsigned int fat_sect = fat_start + (fat_offset / 512);
    unsigned int second_fat_sect = second_fat_start + (fat_offset / 512);
    unsigned int sect_offset = fat_offset % 512;
    fseek(fp, fat_sect * 512, SEEK_SET);
    fread(fat, 2, 512, fp);
    if (fs_type == FAT16) *(unsigned short *) &fat[sect_offset] = val;
    else {
        int odd = first_clustno & 1;
        if (odd) {
            *(unsigned short *) &fat[sect_offset] = ((val & 0xfff) << 4) | ((*(unsigned short *) &fat[sect_offset]) & 0xf);
        } else {
            *(unsigned short *) &fat[sect_offset] = (val & 0xfff) | (((*(unsigned short *) &fat[sect_offset]) >> 12) << 12);
        }
    }
    fseek(fp, fat_sect * 512, SEEK_SET);
    fwrite(fat, 2, 512, fp);
    fflush(fp);
    fseek(fp, second_fat_sect * 512, SEEK_SET);
    fwrite(fat, 2, 512, fp);
    fflush(fp);
    fseek(fp, pos, SEEK_SET);
    free(fat);
}

static void fix_str_to_fat(char *str, int len)
{
    for (int i = 0; i < len; i++) {
        if (str[i] == 0) str[i] = ' '; // \0用空格替代
        else if (str[i] >= 'a' && str[i] <= 'z') str[i] -= 0x20; // 小写字母用大写字母替代
    }
}

static void fix_name_ext(char *name, char *ext)
{
    fix_str_to_fat(name, 8);
    fix_str_to_fat(ext, 3);
}

static void fix_fullname_to_fat(char *fullname, char *name, char *ext)
{
    int ext_dot_index = -1;
    for (int i = 11; i >= 0; i--) {
        if (fullname[i] == '.') {
            ext_dot_index = i; // 找到最后一个点作为后缀
            break;
        }
    }
    if (ext_dot_index == -1) {
        // 没有后缀
        memcpy(name, fullname, 8);
        memset(ext, ' ', 3);
    } else if (ext_dot_index == 0) {
        // 没有文件名
        memset(name, ' ', 8);
        memcpy(ext, fullname + 1, 3); // 跳过.
    } else {
        memcpy(name, fullname, ext_dot_index);
        memcpy(ext, fullname + ext_dot_index + 1, 3);
    }
    // 再修整一下
    fix_name_ext(name, ext);
}

int ftrm()
{
    FILE *fp = fopen(imgname, "rb+");
    fseek(fp, 512, SEEK_SET);
    unsigned char first_fat[2] = {0, 0};
    fread(first_fat, 2, 1, fp);
    if (first_fat[0] == 0xf0 || first_fat[0] == 0xf8) fs_type = FAT12;
    else if (first_fat[0] == 0xff) fs_type = FAT16;
    else {
        puts("Error: invalid file system");
        return 1;
    }
    int root_start_sec = fs_type == FAT12 ? 19 : 65;
    int root_size = fs_type == FAT12 ? 14 : 32;
    int root_ents = 0;
    fseek(fp, root_start_sec * 512, SEEK_SET);
    fileinfo_t *root_dir = (fileinfo_t *) malloc(root_size * 512);
    fread(root_dir, root_size * 16, sizeof(fileinfo_t), fp);
    for (int i = 0; i < root_size * 16; i++) {
        root_ents = i;
        if (!root_dir[i].name[0]) break;
    }
    char fullname[20];
    char name[8], ext[3];
    memset(fullname, 0, 20);
    memset(name, 0, 8);
    memset(ext, 0, 3);
    strncpy(fullname, filename, 15);
    fix_fullname_to_fat(fullname, name, ext);
    int exist = -1;
    for (int i = 0; i < root_ents; i++) {
        if ((!memcmp(root_dir[i].name, name, 8)) && (!memcmp(root_dir[i].ext, ext, 3))) {
            exist = i;
            break;
        }
    } 
    if (exist == -1) {
        printf("Error: file '%s' not exist", filename);
        return 1;
    }
    root_dir[exist].name[0] = 0xe5;
    if (root_dir[exist].clustno == 0) return 0;
    unsigned short clustno = root_dir[exist].clustno, next_clustno;
    while (1) {
        next_clustno = get_next_clustno(fp, clustno);
        set_nth_clustno(fp, clustno, 0);
        if ((fs_type == FAT12 && next_clustno >= 0xff8) || (fs_type == FAT16 && next_clustno >= 0xfff8)) break;
        clustno = next_clustno;
    }
    fseek(fp, root_start_sec * 512, SEEK_SET);
    fwrite(root_dir, root_size * 16, sizeof(fileinfo_t), fp);
    fflush(fp);
    fclose(fp);
    return 0;
}

int ftrm_main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            print_usage(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-img")) {
            i++;
            if (imgname) { // 在此之前已经有文件名，报错
                printf("Error: multiple image names (the first one is: '%s')", imgname);
                return 1;
            }
            imgname = malloc(strlen(argv[i]) + 5);
            if (!imgname) { // 分配内存失败，报错
                printf("Error: no memory for imgname");
                return 1;
            }
            strcpy(imgname, argv[i]); // 复制一个，不用argv了
        } else {
            if (filename) { // 在此之前已经有文件名，报错
                printf("Error: multiple filenames (the first one is: '%s')", filename);
                return 1;
            }
            filename = malloc(strlen(argv[i]) + 5);
            if (!filename) { // 分配内存失败，报错
                printf("Error: no memory for filename");
                return 1;
            }
            strcpy(filename, argv[i]); // 复制一个，不用argv了
        }
    }
    if (!filename) { // 参数中没有文件名，报错
        printf("Error: filename required\n");
        return 1;
    }
    if (!imgname) { // 参数中没有文件名，报错
        printf("Error: imgname required\n");
        return 1;
    }
    int ret = ftrm();
    free(filename);
    free(imgname);
    return ret;
}

#ifndef STANDALONE
int main(int argc, char **argv)
{
    return ftrm_main(argc, argv);
}
#endif
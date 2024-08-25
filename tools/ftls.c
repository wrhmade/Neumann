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
    unsigned char ord;
    char name1[10];
    unsigned char attr, type, checksum;
    char name2[12];
    unsigned short clustno_lo;
    char name3[4];
} __attribute__((packed)) lfn_entry_t;

typedef struct {
    char name[260]; // prepare for LFN
    unsigned int year, mon, day, hour, min, sec;
    unsigned int size;
    char type;
    unsigned short clustno;
} file_t;

typedef enum FILE_SYSTEM_TYPE {
    FAT12, FAT16
} fs_type_t;

char *imgname;
fs_type_t fs_type;
int more = 0;
char *path, **path_stack;
int path_stack_top = 0;

void print_usage(char *name)
{
    puts("ftls 0.0.1 by foolish-shabby");
    printf("Usage: %s <path> <-img imgname> [-l] [-h]\n", name);
    puts("Valid Options:");
    puts("    -img     Specify the image to copy");
    puts("    -l       Show more information (size, time, etc.)");
    puts("    -h       Show this message");
}

void print_02d(int num)
{
    putchar(num / 10 + '0');
    putchar(num % 10 + '0');
}

void path_parse()
{
    char *level_start = path;
    char *level_end = level_start + 1;
    path_stack = (char **) calloc(strlen(path), sizeof(char *));
    while (*level_end) {
        while (*level_end != '/' && *level_end) {
            level_end++;
        }
        int level_len = level_end - level_start;
        if (level_len == 1) {
            level_start = level_end;
            level_end = level_start + 1;
            continue;
        }
        path_stack[path_stack_top] = calloc(level_len, 1);
        char *p = level_start + 1;
        strncpy(path_stack[path_stack_top], p, level_len - 1);
        if (!strcmp(path_stack[path_stack_top], "..")) {
            free(path_stack[path_stack_top]);
            path_stack_top--;
            if (path_stack_top < 0) path_stack_top = 0;
        } else if (!strcmp(path_stack[path_stack_top], ".")) {
            free(path_stack[path_stack_top]);
        } else path_stack_top++;
        if (!*level_end) break;
        level_start = level_end;
        level_end = level_start + 1;
    }
}

int read_dir_entries(FILE *fp, file_t *files, int start_sect)
{
    int root_size = fs_type == FAT12 ? 14 : 32;
    int root_ents = 0;
    fseek(fp, start_sect * 512, SEEK_SET);
    fileinfo_t *root_dir = (fileinfo_t *) calloc(root_size, 512);
    fread(root_dir, root_size * 16, sizeof(fileinfo_t), fp);
    for (int i = 0; i < root_size * 16; i++) {
        root_ents = i;
        if (!root_dir[i].name[0]) break;
    }
    int i_ = 0;
    for (int i = 0; i < root_ents; i++) {
        int ind = 0;
        if (root_dir[i].type == 0x0f) {
            // lfn
            char temp[260 - 13] = {0};
            memcpy(temp, files[i_].name, 260 - 13);
            memcpy(files[i_].name + 13, temp, 260 - 13);
            lfn_entry_t *plfn = (lfn_entry_t *) &root_dir[i];
            lfn_entry_t lfn = *plfn;
            if (lfn.ord == 0xe5) continue;
            for (int j = 0; j < 5; j++) files[i_].name[j] = lfn.name1[j * 2];
            for (int j = 0; j < 6; j++) files[i_].name[j + 5] = lfn.name2[j * 2];
            for (int j = 0; j < 2; j++) files[i_].name[j + 11] = lfn.name3[j * 2];
            continue;
        }
        if (!files[i_].name[0]) {
            for (int j = 0; j < 8; ind++, j++) {
                if (root_dir[i].name[j] == ' ') break;
                files[i_].name[ind] = root_dir[i].name[j];
                if (files[i_].name[ind] >= 'A' && files[i_].name[ind] <= 'Z') files[i_].name[ind] += 0x20;
            }
            if (root_dir[i].ext[0] == ' ' && root_dir[i].ext[1] == ' ' && root_dir[i].ext[2] == ' ') files[i_].name[ind++] = '\0';
            else files[i_].name[ind++] = '.';
            for (int j = 0; j < 3; ind++, j++) {
                if (root_dir[i].ext[j] == ' ') break;
                files[i_].name[ind] = root_dir[i].ext[j];
                if (files[i_].name[ind] >= 'A' && files[i_].name[ind] <= 'Z') files[i_].name[ind] += 0x20;
            }
        }
        files[i_].size = root_dir[i].size;
        files[i_].year = ((root_dir[i].date & 0xfe00) >> 9) + 1980;
        files[i_].mon = (root_dir[i].date & 0x01e0) >> 5;
        files[i_].day = root_dir[i].date & 0x001f;
        files[i_].hour = (root_dir[i].time & 0xf800) >> 11;
        files[i_].min = (root_dir[i].time & 0x07e0) >> 5;
        files[i_].sec = root_dir[i].time & 0x001f;
        files[i_].type = root_dir[i].type;
        files[i_].clustno = root_dir[i].clustno;
        if (strcmp(files[i_].name, ".") && strcmp(files[i_].name, "..")) i_++;
    }
    int actual_root_ents = i_;
    for (int i = 0; i < actual_root_ents - 1; i++) {
        for (int j = 0; j < actual_root_ents - 1 - i; j++) {
            if (strcmp(files[j].name, files[j + 1].name) > 0) {
                file_t temp = files[j];
                files[j] = files[j + 1];
                files[j + 1] = temp;
            }
        }
    }
    free(root_dir);
    return actual_root_ents;
}
  
int ftls()
{
    FILE *fp = fopen(imgname, "rb");
    if (!fp) {
        printf("Error: image '%s' does not exist\n", imgname);
        return 1;
    }
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
    file_t *files = (file_t *) calloc(root_size, 512);
    int root_ents = read_dir_entries(fp, files, root_start_sec);
    path_parse();
    for (int i = 0; i < path_stack_top; i++) {
        file_t *file = NULL;
        for (int j = 0; j < root_ents; j++) {
            if (!strcmp(files[j].name, path_stack[i])) {
                file = &files[j];
                break;
            }
        }
        if (!file) {
            printf("Error: no file or directory '%s' under path /", path_stack[i]);
            for (int j = 0; j < i; j++) {
                printf("%s/", path_stack[j]);
            }
            printf("\n");
            return 1;
        }
        if (file->type == 0x10) {
            unsigned short clustno = file->clustno;
            memset(files, 0, root_size * 512);
            root_ents = read_dir_entries(fp, files, root_start_sec + root_size + clustno - 2);
        } else {
            if (i != path_stack_top - 1) {
                printf("Error: path ");
                for (int j = 0; j <= i; j++) {
                    printf("/%s", path_stack[j]);
                }
                printf("is a file\n");
                return 1;
            }
            file_t f = *file;
            memset(files, 0, root_size * 512);
            files[0] = f;
            root_ents = 1;
        }
    }
    int max_filename_len = -1;
    for (int i = 0; i < root_ents; i++) {
        int cur_len = strlen(files[i].name);
        max_filename_len = max_filename_len > cur_len ? max_filename_len : cur_len;
    }
    for (int i = 0; i < root_ents; i++) {
        if (((unsigned char) files[i].name[0]) == 0xe5) continue;
        printf("%s", files[i].name);
        if (more) {
            int len = max_filename_len - strlen(files[i].name);
            while (len--) printf(" ");
            printf("    ");
            if (files[i].type == 0x10) printf("<DIR>     ");
            else printf("<FILE>    ");
            printf("%d-", files[i].year);
            print_02d(files[i].mon);
            printf("-");
            print_02d(files[i].day);
            printf(" ");
            print_02d(files[i].hour);
            printf(":");
            print_02d(files[i].min);
            printf(":");
            print_02d(files[i].sec);
            printf("     %d Bytes", files[i].size);
        }
        printf("\n");
    }
    free(files);
    for (int i = 0; i < path_stack_top; i++) free(path_stack[i]);
    free(path_stack);
    fclose(fp);
    return 0;
}

int ftls_main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h")) {
            print_usage(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-l")) {
            more = 1;
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
            if (path) { // 在此之前已经有文件名，报错
                printf("Error: multiple paths (the first one is: '%s')", path);
                return 1;
            }
            path = malloc(strlen(argv[i]) + 5);
            if (!path) { // 分配内存失败，报错
                printf("Error: no memory for path");
                return 1;
            }
            strcpy(path, argv[i]); // 复制一个，不用argv了
        }
    }
    if (!path) { // 参数中没有文件名，报错
        path = calloc(1, 5);
        path[0] = '/';
    }
    if (!imgname) {
        printf("Error: imgname required\n");
        return 1;
    }
    int ret = ftls();
    free(path);
    return ret;
}

#ifndef STANDALONE
int main(int argc, char **argv)
{
    return ftls_main(argc, argv);
}
#endif
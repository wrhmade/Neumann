// Author: foolish-shabby <2276316223@qq.com>
// License: WTFPL
// This software is a part of free toolpack 'myfattools', which is aimed to operate FAT images in CLI.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define malloc(size) calloc(size, 1)

char *imgname, *path;
int more = 0;
FILE *image;
char **path_stack;
int path_stack_top = 0;

// FAT16基础实现部分
#define SECTOR_SIZE 512
#define FAT1_SECTORS 32
#define ROOT_DIR_SECTORS 32
#define FAT1_START_LBA 1
#define ROOT_DIR_START_LBA 65
#define DATA_START_LBA 97
#define SECTOR_CLUSTER_BALANCE (DATA_START_LBA - 2)
#define MAX_FILE_NUM 512

typedef struct FILEINFO {
    uint8_t name[8], ext[3];
    uint8_t type, reserved[10];
    uint16_t time, date, clustno;
    uint32_t size;
}  __attribute__((packed)) fileinfo_t;

void hd_read(int lba, int numsects, void *buffer)
{
    fseek(image, lba * 512, SEEK_SET);
    fread(buffer, numsects * 512, 1, image);
}

void hd_write(int lba, int numsects, void *buffer)
{
    fseek(image, lba * 512, SEEK_SET);
    fwrite(buffer, numsects * 512, 1, image);
}

void print_02d(int num)
{
    putchar(num / 10 + '0');
    putchar(num % 10 + '0');
}

// 把原文件名改编为FAT16所要求的8.3格式
int lfn2sfn(const char *lfn, char *sfn)
{
    int len = strlen(lfn), last_dot = -1;
    for (int i = len - 1; i >= 0; i--) { // 从尾到头遍历，寻找最后一个.的位置
        if (lfn[i] == '.') { // 找到了
            last_dot = i; // 最后一个.赋值一下
            break; // 跳出循环
        }
    }
    if (last_dot == -1) last_dot = len; // 没有扩展名，那就在最后虚空加个.
    if (lfn[0] == '.') return -1; // 首字符是.，不支持
    int len_name = last_dot, len_ext = len - 1 - last_dot; // 计算文件名与扩展名各自有多长
    if (len_name > 8) return -1; // 文件名长于8个字符，不支持
    if (len_ext > 3) return -1; // 扩展名长于3个字符，不支持
    // 事实上FAT对此有解决方案，称为长文件名（LFN），但实现较为复杂，暂时先不讨论
    char *name = (char *) malloc(10); // 多分配点内存
    char *ext = NULL; // ext不一定有
    if (len_ext > 0) ext = (char *) malloc(5); // 有扩展名，分配内存
    memcpy(name, lfn, len_name); // 把name从lfn中拷出来
    if (ext) memcpy(ext, lfn + last_dot + 1, len_ext); // 把ext从lfn中拷出来
    if (name[0] == 0xe5) name[0] = 0x05; // 如果第一个字节恰好是0xe5（已删除），将其更换为0x05
    for (int i = 0; i < len_name; i++) { // 处理文件名
        if (name[i] == '.') return -1; // 文件名中含有.，不支持
        if ((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= '0' && name[i] <= '9')) sfn[i] = name[i]; // 数字或字母留为原样
        else sfn[i] = '_'; // 其余字符变为下划线
        if (sfn[i] >= 'a' && sfn[i] <= 'z') sfn[i] -= 0x20; // 小写变大写
    }
    for (int i = len_name; i < 8; i++) sfn[i] = ' '; // 用空格填充剩余部分
    for (int i = 0; i < len_ext; i++) { // 处理扩展名
        if ((ext[i] >= 'a' && ext[i] <= 'z') || (ext[i] >= 'A' && name[i] <= 'Z') || (ext[i] >= '0' && ext[i] <= '9')) sfn[i + 8] = ext[i]; // 数字或字母留为原样
        else sfn[i + 8] = '_'; // 其余字符变为下划线
        if (sfn[i + 8] >= 'a' && sfn[i + 8] <= 'z') sfn[i + 8] -= 0x20; // 小写变大写
    }
    if (len_ext > 0) {
        for (int i = len_ext; i < 3; i++) sfn[i + 8] = ' '; // 用空格填充剩余部分
    } else {
        for (int i = 0; i < 3; i++) sfn[i + 8] = ' '; // 用空格填充剩余部分
    }
    sfn[11] = 0; // 文件名的结尾加一个\0
    return 0; // 正常退出
}

// 读取根目录目录项
fileinfo_t *read_dir_entries(int dir_start_lba, int *dir_ents)
{
    fileinfo_t *root_dir = (fileinfo_t *) malloc(ROOT_DIR_SECTORS * SECTOR_SIZE);
    hd_read(dir_start_lba, ROOT_DIR_SECTORS, root_dir);
    int i;
    for (i = 0; i < MAX_FILE_NUM; i++) {
        if (root_dir[i].name[0] == 0) break; 
    }
    *dir_ents = i;
    return root_dir;
}

void print_usage(char *name)
{
    puts("ftls 0.0.1 by foolish-shabby");
    printf("Usage: %s <imgname> <-path pathname> [-l] [-h]\n", name);
    puts("Valid Options:");
    puts("    -l          Show more details");
    puts("    -path       Specify the path to list out");
    puts("    -img        Specify the image to operate");
    puts("    -h          Show this message");
}

void path_parse()
{
    path_stack = (char **) calloc(strlen(path), sizeof(char *)); // 初始化栈
    if (path[0] != '/') { // 第一个不是/，对后续处理会有影响
        char *new_path = (char *) malloc(strlen(path) + 5); // 从今天起你就是新的path了
        strcpy(new_path, "/"); // 先复制一个/
        strcat(new_path, path); // 再把后续的路径拼接上
        free(path); // 释放原本的路径
        path = new_path; // 夺舍
    }
    char *level_start = path; // 当前路径层级的起始
    char *level_end = level_start + 1; // 当前路径层级的结尾
    while (*level_end) { // 直到还没到结尾
        while (*level_end != '/' && *level_end) {
            level_end++; // 遍历直到抵达`/`
        }
        int level_len = level_end - level_start; // 这一级路径的长度（前/计后/不计）
        if (level_len == 1) { // 如果就只有后面的一个/
            level_start = level_end; // start变为现在的end
            level_end = level_start + 1; // end变为现在的start+1
            continue; // 下一层
        }
        path_stack[path_stack_top] = calloc(level_len, 1); // 初始化这一层路径栈
        char *p = level_start + 1; // 跳过本层路径一开始的/
        strncpy(path_stack[path_stack_top], p, level_len - 1); // 将本层路径拷入路径栈，只拷level_len - 1（去掉一开头的/）的长度
        if (!strcmp(path_stack[path_stack_top], "..")) { // 如果是..
            free(path_stack[path_stack_top]); // 首先释放这一层
            path_stack_top--; // 然后弹栈
            free(path_stack[path_stack_top]); // 然后旧的那一层也就可以释放了
            if (path_stack_top < 0) path_stack_top = 0; // 如果都弹到结尾了，那你还真是nb，避免溢出
        } else if (!strcmp(path_stack[path_stack_top], ".")) {
            free(path_stack[path_stack_top]); // 如果是.，那就相当于白压了，释放即可
        } else path_stack_top++; // 否则就正常入栈
        if (!*level_end) break; // 如果已经到达结尾，直接break，不要指望一开始的while
        level_start = level_end; // start变为现在的end
        level_end = level_start + 1; // end变为start+1
    }
}

int ftls()
{
    image = fopen(imgname, "rb");
    if (!image) {
        puts("Error: image does not exist");
        return 1;
    }
    path_parse();
    int entries = 0;
    fileinfo_t *dir_entries = read_dir_entries(ROOT_DIR_START_LBA, &entries); // 读取目录项，从根目录开始
    char sfn[20] = {0};
    int current_layer_directory_clustno = ROOT_DIR_START_LBA - SECTOR_CLUSTER_BALANCE; // 起始目录为根目录
    int parent_directory_clustno = -1;
    int ftls_status = 0;
    for (int current_layer = 0; current_layer < path_stack_top; current_layer++) {
        memset(sfn, 0, sizeof(sfn));
        int need_lfn_entry = lfn2sfn(path_stack[current_layer], sfn);
        if (need_lfn_entry == -1) {
            printf("Error: path ");
            for (int j = 0; j < current_layer; j++) printf("/%s", path_stack[j]);
            printf("/`%s` needs LFN feature which is unsupported", path_stack[current_layer]);
            ftls_status = -1;
            goto end;
        }
        int if_exists_then_index_else_negative_one = -1;
        for (int j = 0; j < entries; j++) {
            if (!memcmp(dir_entries[j].name, sfn, 8) && !memcmp(dir_entries[j].ext, sfn + 8, 3)) {
                if_exists_then_index_else_negative_one = j; // 找到了
                break;
            }
        }
        if (if_exists_then_index_else_negative_one != -1) { // 找到了文件
            // 这个时候再处理
            fileinfo_t *finfo = &dir_entries[if_exists_then_index_else_negative_one];
            if (current_layer != path_stack_top - 1 && !(finfo->type & 0x10)) { // 不是最后一层不是目录
                printf("Error: path ");
                for (int j = 0; j < current_layer; j++) printf("/%s", path_stack[j]);
                printf("/`%s` is a file, but it should be a directory", path_stack[current_layer]);
                ftls_status = -1;
                goto end;
            }
            if (current_layer != path_stack_top - 1) {
                // 不是最后一层，那只能是目录，则进入之
                parent_directory_clustno = current_layer_directory_clustno;
                current_layer_directory_clustno = dir_entries[if_exists_then_index_else_negative_one].clustno;
                free(dir_entries);
                dir_entries = read_dir_entries(current_layer_directory_clustno + SECTOR_CLUSTER_BALANCE, &entries);
            } else {
                // 是最后一层
                if (finfo->type & 0x10) {
                    // 是目录，则进入之
                    parent_directory_clustno = current_layer_directory_clustno;
                    current_layer_directory_clustno = dir_entries[if_exists_then_index_else_negative_one].clustno;
                    free(dir_entries);
                    dir_entries = read_dir_entries(current_layer_directory_clustno + SECTOR_CLUSTER_BALANCE, &entries);
                } else {
                    // 是最后一层，且是文件
                    fileinfo_t finfo_real = *finfo; // 复制一份
                    free(dir_entries);
                    dir_entries = (fileinfo_t *) malloc(2 * sizeof(fileinfo_t));
                    dir_entries[0] = finfo_real; // 写入，假装这个目录只有这一个文件
                    entries = 1;
                }
            }
        } else {
            printf("Error: path ");
            for (int j = 0; j < current_layer; j++) printf("/%s", path_stack[j]);
            printf("/`%s` is not exist", path_stack[current_layer]);
            ftls_status = -1;
            goto end;
        }
    }
    // 至循环结束，dir_entries应该就是想要访问的目录下所有文件/想要访问的文件本身，开始输出
    // 反正最多512个文件，先做一个最简单的冒泡把目录项按字典序排序
    for (int i = 0; i < entries - 1; i++) {
        for (int j = 0; j < entries - 1 - i; j++) {
            if (strcmp(dir_entries[j].name, dir_entries[j + 1].name) > 0) {
                fileinfo_t temp = dir_entries[j];
                dir_entries[j] = dir_entries[j + 1];
                dir_entries[j + 1] = temp;
            }
        }
    }
    for (int i = 0; i < entries; i++) {
        // 跳过已删除，.和..
        if (dir_entries[i].name[0] == 0xe5) continue;
        // 由于带.文件不会在只支持sfn的FAT文件系统中出现，故可直接判断第一个字节是不是.
        // -l需要显示这俩不能跳过
        if (dir_entries[i].name[0] == '.' && !more) continue;
        int len = 0;
        for (int j = 0; j < 8; j++) {
            if (dir_entries[i].name[j] == ' ') break;
            if (isalpha(dir_entries[i].name[j])) putchar(tolower(dir_entries[i].name[j]));
            else putchar(dir_entries[i].name[j]);
            len++;
        }
        if (dir_entries[i].ext[0] != ' ') {
            putchar('.'); len++;
            for (int j = 0; j < 3; j++) {
                if (dir_entries[i].ext[j] == ' ') break;
                if (isalpha(dir_entries[i].ext[j])) putchar(tolower(dir_entries[i].ext[j]));
                else putchar(dir_entries[i].ext[j]);
                len++;
            }
        }
        if (more) {
            for (int i = len; i <= 12; i++) putchar(' ');
            if (dir_entries[i].type & 0x10) printf("<DIR>      ");
            else printf("<FILE>     ");
            printf("%d-", ((dir_entries[i].date & 0xfe00) >> 9) + 1980); // year
            print_02d((dir_entries[i].date & 0x01e0) >> 5); // month
            putchar('-');
            print_02d(dir_entries[i].date & 0x001f); // day
            putchar(' ');
            print_02d((dir_entries[i].time & 0xf800) >> 11); // hour
            putchar(':');
            print_02d((dir_entries[i].time & 0x07e0) >> 5); // min
            putchar(':');
            print_02d(dir_entries[i].time & 0x001f); // sec
            if (!(dir_entries[i].type & 0x10)) printf("      %d Bytes", dir_entries[i].size);
        }
        putchar('\n');
    }
end:
    return ftls_status;
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
        } else if (!strcmp(argv[i], "-path")) {
            i++;
            if (path) {
                printf("Error: multiple paths to list out (the first one is: '%s')", path);
                return 1;
            }
            path = malloc(strlen(argv[i]) + 5);
            strcpy(path, argv[i]);
        } else if (!strcmp(argv[i], "-l")) {
            more = 1;
        } else if (argv[i][0] == '-') {
            printf("Error: invalid option: %s\n", argv[i]);
            puts("Sorry if that's your imgname.");
            return 1;
        } else {
            if (imgname) { // 在此之前已经有文件名，报错
                printf("Error: multiple imgnames (the first one is: '%s')", imgname);
                return 1;
            }
            imgname = malloc(strlen(argv[i]) + 5);
            if (!imgname) { // 分配内存失败，报错
                printf("Error: no memory for imgname");
                return 1;
            }
            strcpy(imgname, argv[i]); // 复制一个，不用argv了
        }
    }
    if (!imgname) { // 参数中没有文件名，报错
        printf("Error: imgname required\n");
        return 1;
    }
    if (!path) {
        path = malloc(5);
        path[0] = '/';
    }
    int ret = ftls();
    return ret;
}
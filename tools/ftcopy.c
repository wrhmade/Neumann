// Author: foolish-shabby <2276316223@qq.com>
// License: WTFPL
// This software is a part of free toolpack 'myfattools', which is aimed to operate FAT images in CLI.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define malloc(size) calloc((size + 511) / 512 * 512, 1)

char *imgname, *srcpath, *dstpath;
int from = -1;
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
    fflush(image);
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

// 获取第n个FAT项
static uint16_t get_nth_fat(uint16_t n)
{
    uint8_t *fat = (uint8_t *) malloc(512); // 分配临时FAT内存
    uint32_t fat_start = FAT1_START_LBA; // 默认从FAT1中读取FAT
    uint32_t fat_offset = n * 2; // FAT项在FAT表内的偏移，FAT16一个FAT是16位，即2个字节，所以乘2
    uint32_t fat_sect = fat_start + (fat_offset / 512); // 该FAT项对应的扇区编号
    uint32_t sect_offset = fat_offset % 512; // 该FAT项在扇区内的偏移
    hd_read(fat_sect, 1, fat); // 读取对应的一个扇区到FAT内（由于*2，FAT项必然不跨扇区）
    uint16_t table_val = *(uint16_t *) &fat[sect_offset]; // 从FAT表中找到对应的FAT项
    free(fat); // 临时FAT表就用不上了
    return table_val; // 返回对应的FAT项
}

// 设置第n个FAT项
static void set_nth_fat(uint16_t n, uint16_t val)
{
    int fat_start = FAT1_START_LBA; // FAT1起始扇区
    int second_fat_start = FAT1_START_LBA + FAT1_SECTORS; // FAT2起始扇区
    uint8_t *fat = (uint8_t *) malloc(512); // 临时FAT表
    uint32_t fat_offset = n * 2; // FAT项在FAT表内的偏移
    uint32_t fat_sect = fat_start + (fat_offset / 512); // FAT项在FAT1中对应的扇区号
    uint32_t second_fat_sect = second_fat_start + (fat_offset / 512); // FAT项在FAT2中对应的扇区号
    uint32_t sect_offset = fat_offset % 512; // FAT项在扇区内的偏移
    hd_read(fat_sect, 1, fat); // 读入到临时FAT表
    *(uint16_t *) &fat[sect_offset] = val; // 直接设置对应的FAT项即可，FAT16没有那么多弯弯绕
    hd_write(fat_sect, 1, fat); // 写入FAT1
    hd_write(second_fat_sect, 1, fat); // 写入FAT2
    free(fat); // 释放临时FAT表
}

// 读取第n个clust
static void read_nth_clust(uint16_t n, void *clust)
{
    hd_read(n + SECTOR_CLUSTER_BALANCE, 1, clust);
}

// 写入第n个clust
static void write_nth_clust(uint16_t n, const void *clust)
{
    hd_write(n + SECTOR_CLUSTER_BALANCE, 1, (void *) clust);
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

// 创建文件
int fat16_create_file(fileinfo_t *finfo, char *filename, int in_which_dir_clustno)
{
    if (in_which_dir_clustno == 0) in_which_dir_clustno = ROOT_DIR_START_LBA - SECTOR_CLUSTER_BALANCE;
    if (filename[0] == 0xe5) filename[0] = 0x05; // 如上，若第一个字节为 0xe5，需要更换为 0x05
    char sfn[20] = {0};
    int ret = lfn2sfn(filename, sfn); // 将文件名转换为8.3文件名
    if (ret) return -1; // 文件名不符合8.3规范，返回
    int entries;
    fileinfo_t *root_dir = read_dir_entries(in_which_dir_clustno + SECTOR_CLUSTER_BALANCE, &entries); // 读取所有根目录项
    int free_slot = entries; // 默认的空闲位置是最后一个
    for (int i = 0; i < entries; i++) {
        if (!memcmp(root_dir[i].name, sfn, 8) && !memcmp(root_dir[i].ext, sfn + 8, 3)) { // 文件名和扩展名都一样
            free(root_dir); // 已经有了就不用创建了
            return -1;
        }
        if (root_dir[i].name[0] == 0xe5) { // 已经删除（文件名第一个字节是0xe5）
            free_slot = i; // 那就把这里当成空闲位置
            break;
        }
    }
    if ((in_which_dir_clustno == ROOT_DIR_START_LBA - SECTOR_CLUSTER_BALANCE && free_slot == MAX_FILE_NUM)
        || (in_which_dir_clustno != ROOT_DIR_START_LBA - SECTOR_CLUSTER_BALANCE && free_slot == SECTOR_SIZE / sizeof(fileinfo_t))) { // 如果空闲位置已经到达目录末尾
        // 我们把根目录以外的目录限制在一个扇区以内
        free(root_dir); // 没地方创建也就不用创建了
        return -1;
    }
    // 开始填入fileinfo_t对应的项
    memcpy(root_dir[free_slot].name, sfn, 8); // sfn为name与ext的合体，前8个字节是name
    memcpy(root_dir[free_slot].ext, sfn + 8, 3); // 后3个字节是ext
    root_dir[free_slot].type = 0x20; // 类型为0x20（正常文件）
    root_dir[free_slot].clustno = 0; // 没有内容，所以没有簇号（同样放在下一节讲）
    root_dir[free_slot].size = 0; // 没有内容，所以大小为0
    memset(root_dir[free_slot].reserved, 0, 10); // 将预留部分全部设为0
    // 获取当前日期和时间
    time_t tim = time(0);
    struct tm *local = localtime(&tim);
    // 更新日期和时间
    root_dir[free_slot].date = ((local->tm_year + 1900 - 1980) << 9) | ((local->tm_mon + 1) << 5) | local->tm_mday;
    root_dir[free_slot].time = (local->tm_hour << 11) | (local->tm_min << 5) | local->tm_sec;
    if (finfo) *finfo = root_dir[free_slot]; // 创建完了不能不管，传给finfo留着
    hd_write(in_which_dir_clustno + SECTOR_CLUSTER_BALANCE, ((entries + 1) * 32 + 511) / 512, root_dir); // 将新的根目录区写回硬盘
    free(root_dir); // 成功完成
    return 0;
}

// 写入文件，为简单起见相当于覆盖了
int fat16_write_file(fileinfo_t *finfo, int pdir_clustno, const void *buf, uint32_t size)
{
    uint16_t clustno = finfo->clustno, next_clustno; // 从已有首簇号开始
    if (finfo->size == 0 && finfo->clustno == 0) { // 没有首簇号
        clustno = 2; // 从第2个簇开始分配
        while (1) {
            if (get_nth_fat(clustno) == 0) { // 当前簇空闲
                finfo->clustno = clustno; // 分配
                break; // 已找到空闲簇号
            }
            clustno++; // 继续寻找下一个簇
        }
    }
    finfo->size = size; // 更新大小
    int write_sects = (size + 511) / 512; // 确认要写入的扇区总数，这里向上舍入
    while (write_sects) { // 只要还要写
        write_nth_clust(clustno, buf); // 将当前buf的512字节写入对应簇中
        write_sects--; // 要写入扇区总数-1
        buf += 512; // buf后移一个扇区
        next_clustno = get_nth_fat(clustno); // 寻找下一个簇
        if (next_clustno == 0 || next_clustno >= 0xfff8) {
            // 当前簇不可用
            next_clustno = clustno + 1; // 从下一个簇开始
            while (1) {
                if (get_nth_fat(next_clustno) == 0) { // 这个簇是可用的
                    set_nth_fat(clustno, next_clustno); // 将这个簇当成下一个簇链接上去
                    break;
                } else next_clustno++; // 否则，只好继续了
            }
        }
        clustno = next_clustno; // 将下一个簇看做当前簇
    }
    next_clustno = get_nth_fat(clustno); // 获取最后一个簇的簇号
    set_nth_fat(clustno, 0xffff); // 文件结束
    // 释放可能存在的多余簇号
    if (next_clustno > 1 && next_clustno < 0xfff0) { // 若尚有簇未释放
        while (1) {
            clustno = next_clustno; // 将下一个簇视作当前簇
            next_clustno = get_nth_fat(clustno); // 获取下一个簇的簇号
            set_nth_fat(clustno, 0); // 将当前簇号设为0
            if (next_clustno >= 0xfff8) break; // 用完了
        }
    }
    // 获取当前日期和时间
    time_t tim = time(0);
    struct tm *local = localtime(&tim);
    // 更新日期和时间
    finfo->date = ((local->tm_year + 1900 - 1980) << 9) | ((local->tm_mon + 1) << 5) | local->tm_mday;
    finfo->time = (local->tm_hour << 11) | (local->tm_min << 5) | local->tm_sec;
    int entries;
    if (pdir_clustno == 0) pdir_clustno = ROOT_DIR_START_LBA - SECTOR_CLUSTER_BALANCE;
    fileinfo_t *root_dir = read_dir_entries(pdir_clustno + SECTOR_CLUSTER_BALANCE, &entries);
    for (int i = 0; i < entries; i++) {
        if (!memcmp(root_dir[i].name, finfo->name, 8) && !memcmp(root_dir[i].ext, finfo->ext, 3)) {
            root_dir[i] = *finfo; // 找到对应的文件，写进根目录
            break;
        }
    }
    hd_write(pdir_clustno + SECTOR_CLUSTER_BALANCE, ROOT_DIR_SECTORS, root_dir); // 同步到硬盘
    free(root_dir);
    return 0;
}

void print_usage(char *name)
{
    puts("ftcopy 0.0.1 by foolish-shabby");
    printf("Usage: %s <imgname> <-srcpath pathname> <-dstpath pathname> <-to|-from> [-h]\n", name);
    puts("Valid Options:");
    puts("    -to         Copy the file in the path into the image");
    puts("    -from       Copy the file in the path out of the image");
    puts("    -srcpath    Specify the source of the file");
    puts("    -dstpath    Specify the destination of the file");
    puts("    -img        Specify the image to operate");
    puts("    -h          Show this message");
}

void path_parse()
{
    char *path; // 要解析哪个路径呢？总之是要解析软盘里面的，软盘外面的路径交给OS
    if (from) path = srcpath; 
    else path = dstpath;
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

int ftcopy_from()
{
    image = fopen(imgname, "rb+"); // 打开image，这样hd_read一系列函数均可用
    if (!image) {
        puts("Error: disk image does not exist");
        return 1;
    }
    path_parse(); // 然后执行路径解析
    int entries = 0;
    fileinfo_t *dir_entries = read_dir_entries(ROOT_DIR_START_LBA, &entries); // 读取目录项，从根目录开始
    char sfn[20] = {0}; // sfn临时缓冲区
    int clustno = 0, size = -1; // 文件的簇号和大小
    for (int i = 0; i < path_stack_top; i++) { // 对每一层路径依次进行判断
        int need_lfn_entry = lfn2sfn(path_stack[i], sfn); // 先转化为sfn
        if (need_lfn_entry == -1) { // 不能转化
            printf("Error: `%s` is invalid as a SFN, but a LFN which is unsupported\n", path_stack[i]); // 报错
            return 1;
        }
        int found = 0; // 这一层目录里是否存在要找的目录项？
        for (int j = 0; j < entries; j++) { // 遍历目录项
            if (!memcmp(sfn, dir_entries[j].name, 8) && !memcmp(sfn + 8, dir_entries[j].ext, 3)) { // 名字一样就是找到了
                found = 1;
                if ((i == path_stack_top - 1) && (dir_entries[j].type == 0x10)) { // 最后一层路径是目录
                    printf("Error: path `");
                    for (int k = 0; k <= i; k++) {
                        printf("/%s", path_stack[k]);
                    }
                    puts("` should be a regular file, but a directory instead");
                    return 1;
                } else if ((i != path_stack_top - 1) && (dir_entries[j].type != 0x10)) { // 中间某层路径不是目录
                    printf("Error: path `");
                    for (int k = 0; k <= i; k++) {
                        printf("/%s", path_stack[k]);
                    }
                    puts("` should be a directory, but a regular file instead");
                    return 1;
                }
                clustno = dir_entries[j].clustno; // 获取当前目录项的簇号
                if (i != path_stack_top - 1) { // 如果还是目录
                    free(dir_entries);
                    dir_entries = read_dir_entries(clustno + SECTOR_CLUSTER_BALANCE, &entries); // 重新读取目录项
                    // FAT文件系统的目录是一种特殊的文件，其内容为一堆目录项，与根目录区格式相同
                    clustno = 0; // 簇号就是没有
                    break; // 那就不再找了
                }
                size = dir_entries[j].size; // 不是目录，那就是普通文件，读出大小
            }
        }
        if (!found) { // 没找到，报错
            printf("Error: path `");
            for (int k = 0; k <= i; k++) {
                printf("/%s", path_stack[k]);
            }
            puts("` does not exist");
            return 1;
        }
    }
    free(dir_entries); // 释放临时缓冲区
    FILE *fp = fopen(dstpath, "wb"); // 打开要写入的文件
    if (!fp) { // 不存在，报错
        printf("Error: destination path does not exist\n", dstpath);
        return 1;
    }
    char *clust = (char *) malloc(512); // 单独给簇分配一个缓冲区，直接往buf里写也行
    do {
        read_nth_clust(clustno, clust); // 将该簇号对应的簇读取进来
        fwrite(clust, size >= 512 ? 512 : size, 1, fp); // 写入文件，如果还有一个扇区就写一个扇区，没有一个扇区了就把剩下的写进去
        size -= 512; // 已经写了一个扇区
        fflush(fp); // fwrite完要及时刷新，不然会留在缓冲区里不走，直到fclose了都不一定走
        clustno = get_nth_fat(clustno); // 获取下一个簇号
        if (clustno >= 0xFFF8) break; // 文件结束，退出循环
    } while (1);
    free(clust); // 读完了，释放临时缓冲区
    fclose(fp); // 关闭写入文件
    fclose(image); // 关闭镜像文件
    return 0;
}

int ftcopy_to()
{
    int ftcopy_to_status = 0;
    image = fopen(imgname, "rb+");
    if (!image) {
        puts("Error: disk image does not exist");
        return 1;
    }
    path_parse(); // 然后执行路径解析
    int entries = 0;
    fileinfo_t *dir_entries = read_dir_entries(ROOT_DIR_START_LBA, &entries); // 读取目录项，从根目录开始
    char sfn[20] = {0};
    int current_layer_directory_clustno = ROOT_DIR_START_LBA - SECTOR_CLUSTER_BALANCE; // 起始目录为根目录
    int parent_directory_clustno = -1;
    int final_file_pos = -1;
    for (int current_layer = 0; current_layer < path_stack_top; current_layer++) { // 对每一层路径依次进行判断
        if (current_layer != 0 && entries == SECTOR_SIZE / sizeof(fileinfo_t)) {
            printf("Error: directory ");
            for (int i = 0; i < current_layer; i++) printf("/%s", path_stack[i]);
            printf("/`%s` is full\n", path_stack[current_layer]);
            ftcopy_to_status = -1;
            goto end;
        }
        memset(sfn, 0, sizeof(sfn));
        int need_lfn_entry = lfn2sfn(path_stack[current_layer], sfn);
        if (need_lfn_entry == -1) {
            printf("Error: path ");
            for (int j = 0; j < current_layer; j++) printf("/%s", path_stack[j]);
            printf("/`%s` needs LFN feature which is unsupported", path_stack[current_layer]);
            ftcopy_to_status = -1;
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
            if (current_layer == path_stack_top - 1 && !!(finfo->type & 0x10)) { // 最后一层是目录
                printf("Error: path ");
                for (int j = 0; j < current_layer; j++) printf("/%s", path_stack[j]);
                printf("/`%s` is a directory, but it should be a file", path_stack[current_layer]);
                ftcopy_to_status = -1;
                goto end;
            }
            if (current_layer != path_stack_top - 1 && !(finfo->type & 0x10)) { // 不是最后一层不是目录
                printf("Error: path ");
                for (int j = 0; j < current_layer; j++) printf("/%s", path_stack[j]);
                printf("/`%s` is a file, but it should be a directory", path_stack[current_layer]);
                ftcopy_to_status = -1;
                goto end;
            }
            // 现在要么是最后一层且不是目录，要么不是最后一层且是目录
            if (current_layer != path_stack_top - 1) {
                // 不是最后一层，则进入这个目录
                parent_directory_clustno = current_layer_directory_clustno; // 父目录的簇号为现在的当前目录的簇号
                current_layer_directory_clustno = finfo->clustno; // 当前目录的簇号为这个目录的簇号
                free(dir_entries); // 释放过去的目录项
                dir_entries = read_dir_entries(current_layer_directory_clustno + SECTOR_CLUSTER_BALANCE, &entries); // 将新目录的目录项重新读取
                continue; // 跳过后续处理
            }
            // 是最后一层且不是目录
            // 则此时当前文件的父目录就是现在的目录，直接不处理即可
            final_file_pos = if_exists_then_index_else_negative_one; // 最终文件的索引
        } else { // 不存在文件
            if (current_layer == path_stack_top - 1) {
                // 是最后一层，只需创建文件即可
                int status = fat16_create_file(NULL, path_stack[current_layer], current_layer_directory_clustno);
                // 创建了但是没法知道在哪，所以重新找一遍
                free(dir_entries);
                dir_entries = read_dir_entries(current_layer_directory_clustno + SECTOR_CLUSTER_BALANCE, &entries); // 更新目录项
                for (int j = 0; j < entries; j++) {
                    if (!memcmp(dir_entries[j].name, sfn, 8) && !memcmp(dir_entries[j].ext, sfn + 8, 3)) {
                        final_file_pos = j; // 找到了
                        break;
                    }
                }
            } else {
                // 不是最后一层，还需创建目录
                fileinfo_t directory_creating;
                int status = fat16_create_file(&directory_creating, path_stack[current_layer], current_layer_directory_clustno); // 先当成一个文件去创建
                // 向刚创建的文件中写入一个空扇区（这样做的目的是拿到一个新簇号）
                char *clust = (char *) malloc(512);
                fat16_write_file(&directory_creating, current_layer_directory_clustno, clust, 512);
                free(clust);
                // 待会要把大小改成0，所以要找索引，不过先不着急吧
                fileinfo_t *the_content_of_this_magical_directory_considered_as_file_is_entries = (fileinfo_t *) malloc(5 * sizeof(fileinfo_t));
                strcpy(the_content_of_this_magical_directory_considered_as_file_is_entries[0].name, ".          ");
                strcpy(the_content_of_this_magical_directory_considered_as_file_is_entries[1].name, "..         ");
                the_content_of_this_magical_directory_considered_as_file_is_entries[0].type = 0x10;
                the_content_of_this_magical_directory_considered_as_file_is_entries[1].type = 0x10;
                the_content_of_this_magical_directory_considered_as_file_is_entries[0].time = directory_creating.time;
                the_content_of_this_magical_directory_considered_as_file_is_entries[0].date = directory_creating.date;
                the_content_of_this_magical_directory_considered_as_file_is_entries[1].time = directory_creating.time;
                the_content_of_this_magical_directory_considered_as_file_is_entries[1].date = directory_creating.date;
                the_content_of_this_magical_directory_considered_as_file_is_entries[0].clustno = directory_creating.clustno;
                the_content_of_this_magical_directory_considered_as_file_is_entries[1].clustno = current_layer_directory_clustno;
                the_content_of_this_magical_directory_considered_as_file_is_entries[0].size = 0;
                the_content_of_this_magical_directory_considered_as_file_is_entries[1].size = 0;
                memset(the_content_of_this_magical_directory_considered_as_file_is_entries[0].reserved, 0, 10);
                memset(the_content_of_this_magical_directory_considered_as_file_is_entries[1].reserved, 0, 10);
                // 至此两个目录项创建完成，写入到“文件”当中
                fat16_write_file(&directory_creating, current_layer_directory_clustno, the_content_of_this_magical_directory_considered_as_file_is_entries, 2 * sizeof(fileinfo_t));
                // 此时只要再修改一个type和size这个文件就是目录了，所以找到这个文件并修改之
                int directory_creating_pos = -1;
                free(dir_entries);
                dir_entries = read_dir_entries(current_layer_directory_clustno + SECTOR_CLUSTER_BALANCE, &entries); // 更新目录项
                for (int j = 0; j < entries; j++) {
                    if (!memcmp(dir_entries[j].name, sfn, 8) && !memcmp(dir_entries[j].ext, sfn + 8, 3)) {
                        dir_entries[j].type = 0x10; // 属性为目录
                        dir_entries[j].size = 0; // 大小为0
                        break;
                    }
                }
                hd_write(current_layer_directory_clustno + SECTOR_CLUSTER_BALANCE, (entries * sizeof(fileinfo_t) + 511) / 512, dir_entries);
                // 更新完成，进入这个目录
                parent_directory_clustno = current_layer_directory_clustno;
                current_layer_directory_clustno = directory_creating.clustno;
                free(dir_entries);
                dir_entries = read_dir_entries(current_layer_directory_clustno + SECTOR_CLUSTER_BALANCE, &entries); // 更新目录项
            }
        }
    }
    // 先把文件读到
    FILE *fp = fopen(srcpath, "rb");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = (char *) malloc(size + 5);
    fread(buffer, size, 1, fp);
    fclose(fp);
    // 把文件内容写入进待写的位置
    fat16_write_file(&dir_entries[final_file_pos], current_layer_directory_clustno, buffer, size);
end:
    for (int i = 0; i < path_stack_top; i++) free(path_stack[i]);
    free(path_stack);
    free(dir_entries);
    free(imgname);
    free(srcpath);
    free(dstpath);
    fclose(image);
    return ftcopy_to_status;
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
        } else if (!strcmp(argv[i], "-srcpath")) {
            i++;
            if (srcpath) {
                printf("Error: multiple source paths (the first one is: '%s')", srcpath);
                return 1;
            }
            srcpath = malloc(strlen(argv[i]) + 5);
            strcpy(srcpath, argv[i]);
        } else if (!strcmp(argv[i], "-dstpath")) {
            i++;
            if (dstpath) {
                printf("Error: multiple destination paths (the first one is: '%s')", dstpath);
                return 1;
            }
            dstpath = malloc(strlen(argv[i]) + 5);
            strcpy(dstpath, argv[i]);
        } else if (!strcmp(argv[i], "-from")) {
            from = 1;
        } else if (!strcmp(argv[i], "-to")) {
            from = 0;
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
    if (from == -1) {
        printf("Error: expected '-from' or '-to' in arguments\n");
        return 1;
    }
    if (!srcpath) {
        puts("Error: source path required");
        return 1;
    }
    if (!dstpath) {
        puts("Error: destination path required");
        return 1;
    }
    int ret = from ? ftcopy_from() : ftcopy_to();
    return ret;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

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
    char name[255]; // prepare for LFN
    unsigned int clustno;
    unsigned int size;
    int corr_sfn;
} file_t;

typedef enum FILE_SYSTEM_TYPE {
    FAT12, FAT16
} fs_type_t;

int from = -1;
char *filename;
char *imgname;
fs_type_t fs_type;

void print_usage(char *name)
{
    puts("ftcopy INDEV by foolish-shabby");
    printf("Usage: %s <filename> <-from|-to> <-img imgname> [-h]\n", name);
    puts("Valid options:");
    puts("    -from    Specify to copy out from img");
    puts("    -to      Specify to copy into img");
    puts("    -img     Specify the image to copy");
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

int get_all_clustnos(FILE *fp, unsigned short *clustnos, unsigned short first_clustno)
{
    int cnt = 0;
    *clustnos = first_clustno;
    while (1) {
        first_clustno = get_next_clustno(fp, first_clustno);
        clustnos++;
        *clustnos = first_clustno;
        cnt++;
        if ((fs_type == FAT12 && first_clustno >= 0xff8) || (fs_type == FAT16 && first_clustno >= 0xfff8)) break;
    }
    return cnt;
}

int ftcopy_from()
{
    FILE *fp = fopen(imgname, "rb");
    fseek(fp, 512, SEEK_SET);
    unsigned char first_fat[2] = {0, 0};
    fread(first_fat, 2, 1, fp);
    if (first_fat[0] == 0xf0 || first_fat[0] == 0xf8) fs_type = FAT12;
    else if (first_fat[0] == 0xff) fs_type = FAT16;
    else {
        puts("Error: invalid file system");
        return 1;
    }
    int root_start_sec = fs_type == FAT12 ? 19 : 97;
    int root_size = fs_type == FAT12 ? 14 : 32;
    int root_ents = 0;
    fseek(fp, root_start_sec * 512, SEEK_SET);
    fileinfo_t *root_dir = (fileinfo_t *) malloc(root_size * 512);
    fread(root_dir, root_size * 16, sizeof(fileinfo_t), fp);
    for (int i = 0; i < root_size * 16; i++) {
        root_ents = i;
        if (!root_dir[i].name[0]) break;
    }
    file_t *files = (file_t *) malloc(root_ents * sizeof(file_t));
    memset(files, 0, root_ents * sizeof(file_t));
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
            if (lfn.ord == 0xe5) break;
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
            if (root_dir[i].ext[0] == ' ' && root_dir[i].ext[1] == ' ' && root_dir[i].ext[2] == ' ') files[i_].name[ind++] = ' ';
            else files[i_].name[ind++] = '.';
            for (int j = 0; j < 3; ind++, j++) {
                if (root_dir[i].ext[j] == ' ') break;
                files[i_].name[ind] = root_dir[i].ext[j];
                if (files[i_].name[ind] >= 'A' && files[i_].name[ind] <= 'Z') files[i_].name[ind] += 0x20;
            }
        }
        files[i_].clustno = root_dir[i].clustno;
        files[i_].size = root_dir[i].size;
        files[i_].corr_sfn = i;
        i_++;
    }
    int actual_root_ents = i_;
    file_t file;
    int exists = -1;
    for (int i = 0; i < actual_root_ents; i++) {
        if (!strcmp(files[i].name, filename)) {
            file = files[i];
            exists = i;
            break;
        }
    }
    if (exists == -1) {
        printf("Error: file '%s' is not exist in image '%s'\n", filename, imgname);
        return 1;
    }
    unsigned short *clustnos = (unsigned short *) malloc(0xFFFF * 2);
    int cnt = get_all_clustnos(fp, clustnos, file.clustno);
    char *clust = calloc(512, 1);
    FILE *wrfile = fopen(filename, "wb");
    for (int i = 0; i < cnt; i++) {
        fseek(fp, (root_start_sec + root_size + clustnos[i] - 2) * 512, SEEK_SET);
        fread(clust, 512, 1, fp);
        int bytes = file.size >= 512 ? 512 : file.size;
        fwrite(clust, bytes, 1, wrfile);
        file.size -= 512;
    }
    free(clust);
    free(root_dir);
    free(files);
    fclose(fp);
    fclose(wrfile);
    return 0;
}

int lfn2sfn(const char *lfn, char *sfn)
{
    int len = strlen(lfn), last_dot = -1;
    for (int i = len - 1; i >= 0; i--) {
        if (lfn[i] == '.') {
            last_dot = i;
            break;
        }
    }
    int need_lfn_entry = 0;
    if (lfn[0] == '.') {
        lfn++; 
        if (last_dot == 0) last_dot = len;
        last_dot--;
        if (last_dot == 0) len--;
        need_lfn_entry = 1;
    }
    int len_name = last_dot, len_ext = len - 1 - last_dot;
    if (len_name > 8) len_name = 6, need_lfn_entry = 1;
    if (len_ext > 3) len_name = 6, len_ext = 3, need_lfn_entry = 1;
    char *name = (char *) calloc(len_name + 5, 1);
    char *ext = NULL;
    if (len_ext > 0) ext = (char *) calloc(len_ext + 5, 1);
    memcpy(name, lfn, len_name);
    if (ext) memcpy(ext, lfn + last_dot + 1, len_ext);
    for (int i = 0; i < len_name; i++) {
        if (name[i] == '.') {
            len_name = i;
            need_lfn_entry = 1;
            break;
        }
        if (isalnum(name[i])) sfn[i] = name[i];
        else sfn[i] = '_';
        if (sfn[i] >= 'a' && sfn[i] <= 'z') sfn[i] -= 0x20;
    }
    if (need_lfn_entry) sfn[len_name] = '~', sfn[len_name + 1] = 'x', len_name += 2;
    for (int i = len_name; i < 8; i++) sfn[i] = ' ';
    for (int i = 0; i < len_ext; i++) {
        if (isalnum(ext[i])) sfn[i + 8] = ext[i];
        else sfn[i + 8] = '_';
        if (sfn[i + 8] >= 'a' && sfn[i + 8] <= 'z') sfn[i + 8] -= 0x20;
    }
    for (int i = len_ext; i < 3; i++) sfn[i + 8] = ' ';
    sfn[11] = 0;
    return need_lfn_entry;
}

int ftcopy_to()
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
    file_t *files = (file_t *) calloc(root_ents, sizeof(file_t));
    memset(files, 0, root_ents * sizeof(file_t));
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
            if (lfn.ord == 0xe5) break;
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
            files[i_].name[ind++] = '.';
            for (int j = 0; j < 3; ind++, j++) {
                if (root_dir[i].ext[j] == ' ') break;
                files[i_].name[ind] = root_dir[i].ext[j];
                if (files[i_].name[ind] >= 'A' && files[i_].name[ind] <= 'Z') files[i_].name[ind] += 0x20;
            }
        }
        files[i_].clustno = root_dir[i].clustno;
        files[i_].size = root_dir[i].size;
        files[i_].corr_sfn = i;
        i_++;
    }
    int actual_root_ents = i_;
    FILE *rdfile = fopen(filename, "rb");
    fseek(rdfile, 0, SEEK_END);
    long int fsize = ftell(rdfile);
    fseek(rdfile, 0, SEEK_SET);
    int file_size = (fsize + 511) / 512;
    char *content = calloc(file_size * 512, 1);
    fread(content, 1, fsize, rdfile);
    char sfn[20];
    memset(sfn, 0, 20);
    int len_fn = strlen(filename);
    char *onlyname = (char *) calloc(len_fn + 5, 1);
    int actual_file_name_len = 0;
    int do_have_slash = 0;
    for (int i = len_fn - 1; i >= 0; i--) {
        if (filename[i] == '/') {
            actual_file_name_len = len_fn - i;
            do_have_slash = 1;
            break;
        }
        onlyname[len_fn - 1 - i] = filename[i];
    }
    if (do_have_slash) {
        for (int i = 0; i <= actual_file_name_len / 2; i++) {
            char c = onlyname[actual_file_name_len - 1 - i];
            onlyname[actual_file_name_len - 1 - i] = onlyname[i];
            onlyname[i] = c;
        }
        onlyname[actual_file_name_len] = 0;
        for (int i = 1; i < actual_file_name_len; i++) {
            onlyname[i - 1] = onlyname[i];
        }
        actual_file_name_len--;
        onlyname[actual_file_name_len] = 0;
        free(filename);
        filename = onlyname;
    } else {
        free(onlyname);
    }
    int need_lfn_entry = lfn2sfn(filename, sfn);
    char name[8] = {0}, ext[3] = {0};
    memcpy(name, sfn, 8);
    memcpy(ext, sfn + 8, 3);
    int exist = -1;
    if (need_lfn_entry) {
        for (int i = actual_root_ents - 1; i >= 0; i--) {
            if (!strcmp(files[i].name, filename)) {
                exist = i;
                goto loop;
            }
        }
        int num_ext = -1;
        for (int i = root_ents - 1; i >= 0; i--) {
            int match_s1 = 0, tmark = -1;
            for (int j = 0; j < 8; j++) {
                if (root_dir[i].name[j] == '~') {
                    tmark = j; break;
                }
            }
            tmark++;
            if (tmark && (!strncmp(name, root_dir[i].name, tmark))) match_s1 = 1;
            int match = match_s1 && strcmp(ext, root_dir[i].ext);
            if (exist != i && match) {
                num_ext = root_dir[i].name[tmark] - '0' + 1;
                break;
            }
        }
        if (exist == -1 && num_ext == -1) num_ext = 1;
        int tmark = -1;
        for (int j = 0; j < 8; j++) {
            if (name[j] == '~') {
                tmark = j; break;
            }
        }
        sfn[tmark + 1] = name[tmark + 1] = num_ext + '0';
    } else {
        for (int i = root_ents - 1; i >= 0; i--) {
            if ((!memcmp(root_dir[i].name, name, 8)) && (!memcmp(root_dir[i].ext, ext, 3))) {
                exist = i;
                break;
            }
        }
    }
loop:
    if (exist == -1) {
        int len = strlen(filename);
        for (int i = 0; i < len / 2; i++) {
            char c = filename[i];
            filename[i] = filename[len - 1 - i];
            filename[len - 1 - i] = c;
        }
        char lfn_name[13] = {0};
        if (need_lfn_entry) {
            int times = (len + 12) / 13;
            for (int i = 0; i < times; i++) {
                lfn_entry_t lfn;
                lfn.attr = 0x0f;
                lfn.type = 0;
                lfn.clustno_lo = 0;
                lfn.checksum = 0;
                lfn.ord = times - i;
                if (!i) lfn.ord |= 0x40;
                for (int j = 0; j < 11; j++) {
                    lfn.checksum = (lfn.checksum >> 1) + (lfn.checksum << 7) + sfn[j];
                }
                int actual_len = len - i * 13 < 13 ? len - i * 13 : 13;
                memcpy(lfn_name, filename + i * 13, actual_len);
                for (int j = 0; j < actual_len / 2; j++) {
                    char c = lfn_name[j];
                    lfn_name[j] = lfn_name[actual_len - 1 - j];
                    lfn_name[actual_len - 1 - j] = c;
                }
                for (int j = actual_len; j < 13; j++) {
                    lfn_name[j] = ' ';
                }
                for (int j = 0; j < 5; j++) lfn.name1[j * 2] = lfn_name[j];
                for (int j = 0; j < 6; j++) lfn.name2[j * 2] = lfn_name[j + 5];
                for (int j = 0; j < 2; j++) lfn.name3[j * 2] = lfn_name[j + 11];
                root_dir[root_ents] = *(fileinfo_t *) &lfn;
                root_ents++;
            }
        }
        for (int i = 0; i < len / 2; i++) {
            char c = filename[i];
            filename[i] = filename[len - 1 - i];
            filename[len - 1 - i] = c;
        }
        strncpy(root_dir[root_ents].name, name, 8);
        strncpy(root_dir[root_ents].ext, ext, 3);
        root_dir[root_ents].type = 0x20;
        fseek(fp, root_start_sec * 512, SEEK_SET);
        fwrite(root_dir, root_size * 16, sizeof(fileinfo_t), fp);
        fflush(fp);
    }
    fileinfo_t file = root_dir[exist == -1 ? root_ents : exist];
    unsigned short clustno = file.clustno;
    file.size = fsize;
    if (clustno == 0) {
        clustno = 2;
        while (1) {
            if (!get_next_clustno(fp, clustno)) {
                file.clustno = clustno;
                break;
            } else clustno++;
        }
    }
    int sects = (fsize + 511) / 512;
    int i = 0;
    unsigned short next_clustno;
    while (1) {
        fseek(fp, (root_start_sec + root_size + clustno - 2) * 512, SEEK_SET);
        fwrite(content, 512, 1, fp);
        if (i == sects - 1) break;
        i++;
        content += 512;
        next_clustno = get_next_clustno(fp, clustno);
        if ((next_clustno == 0) || ((fs_type == FAT12 && next_clustno >= 0xff8) || (fs_type == FAT16 && next_clustno >= 0xfff8))) {
            next_clustno = clustno + 1;
            while (1) {
                if (!get_next_clustno(fp, next_clustno)) {
                    set_nth_clustno(fp, clustno, next_clustno);
                    break;
                } else next_clustno++;
            }
        }
        clustno = next_clustno;
    }
    next_clustno = get_next_clustno(fp, clustno);
    set_nth_clustno(fp, clustno, 0xffff);
    if (next_clustno > 1 && ((fs_type == FAT16 && next_clustno < 0xfff0) || (fs_type == FAT12 && next_clustno < 0xff0))) {
        while (1) {
            clustno = next_clustno;
            next_clustno = get_next_clustno(fp, clustno);
            set_nth_clustno(fp, clustno, 0);
            if ((fs_type == FAT12 && next_clustno >= 0xff8) || (fs_type == FAT16 && next_clustno >= 0xfff8)) break;
        }
    }
    time_t timer = time(0);
    struct tm *local = localtime(&timer);
    file.date = ((local->tm_year + 1900 - 1980) << 9) | ((local->tm_mon + 1) << 5) | local->tm_mday;
    file.time = (local->tm_hour << 11) | (local->tm_min << 5) | local->tm_sec;
    root_dir[exist == -1 ? root_ents : exist] = file;
    fseek(fp, root_start_sec * 512, SEEK_SET);
    fwrite(root_dir, root_size * 16, sizeof(fileinfo_t), fp);
    fflush(fp);
    fclose(rdfile);
    fclose(fp);
    return 0;
}

int ftcopy()
{
    if (from) return ftcopy_from();
    return ftcopy_to();
}

int ftcopy_main(int argc, char **argv)
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
        } else if (!strcmp(argv[i], "-from")) {
            if (from != -1) {
                printf("Error: multiple -from or -to");
                return 1;
            }
            from = 1;
        } else if (!strcmp(argv[i], "-to")) {
            if (from != -1) {
                printf("Error: multiple -from or -to");
                return 1;
            }
            from = 0;
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
    if (from == -1) {
        printf("Error: no -from or -to\n");
        return 1;
    }
    int ret = ftcopy();
    free(filename);
    free(imgname);
    return ret;
}

#ifndef STANDALONE
int main(int argc, char **argv)
{
    return ftcopy_main(argc, argv);
}
#endif
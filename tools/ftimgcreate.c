#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum IMAGE_TYPE {
    FD, HD
} im_type_t;

void print_usage(char *name)
{
    puts("ftimgcreate INDEV by foolish-shabby");
    printf("Usage: %s <filename> [-t hd|fd] [-size num] [-h]\n", name);
    puts("Valid Options:");
    puts("    -t       Specify the type of image (fd or hd), 'fd' in default");
    puts("    -size    Specify the size of image, in megabytes (only support hd)");
    puts("    -h       Show this message");
}

char *filename;
im_type_t im_type = FD;
int size = 0;

int ftimgcreate()
{
    FILE *fp = fopen(filename, "wb");
    if (size == 0) {
        printf("Error: no size specified");
        return 1;
    }
    for (int i = 0; i < size; i++) {
        fputc('\0', fp);
    }
    fclose(fp);
    return 0;
}

int ftimgcreate_main(int argc, char **argv)
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
                size = 1474560;
            } else {
                printf("Invalid image type: 'fd' or 'hd' expected, '%s' occurred", argv[i]);
                return 1;
            }
        } else if (!strcmp(argv[i], "-size")) {
            i++;
            if (im_type == FD) {
                printf("Only hard disks can specify its own size");
                return 1;
            }
            else {
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
            }
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
    int ret = ftimgcreate();
    free(filename);
    return ret;
}

#ifndef STANDALONE
int main(int argc, char **argv)
{
    return ftimgcreate_main(argc, argv);
}
#endif
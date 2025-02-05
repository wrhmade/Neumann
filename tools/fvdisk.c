#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//标准扇区数
#define CRITERIA_SECTOR_NUM 512

//标准根目录总项数
#define CRITERIA_ROOT_DIR_NUM 512

//简单向上取整函数
#define EASY_CEIL(n, m) (((n - 1) / m) + 1)

//strupr函数实现
char* strupr(char* str) {
	char *p = str;
	while (*p) {
		*p = toupper((unsigned char)*p);
 		p++;
	}
	return str;
}

//结构体
struct FAT_BPB
{
    unsigned char Jmp_short[3];
    unsigned char BPB_OEMName[8];
    unsigned short BPB_BytesPerSec;
    unsigned char BPB_SecPerClus;
    unsigned short BPB_RsvdSecCnt;
    unsigned char BPB_NumFATs;
    unsigned short BPB_RootEntCnt;
    unsigned short BPB_TotSec16;
    unsigned char BPB_Media;
    unsigned char BPB_FatSz16;
    unsigned short BPB_SecPerTrk;
    unsigned short BPB_NumHeads;
    unsigned int BPB_HiddSec;
    unsigned int BPB_TotSec32;
    unsigned char BS_drvNum;
    unsigned char BS_Reserved1;
    unsigned char BS_BootSig;
    unsigned int BS_VolID;
    unsigned char BS_VolLab[11];
    unsigned char BS_FilSysType[8];
} __attribute__((packed));

struct FAT_START_ADDR
{
    unsigned int dbr_start_addr;
    unsigned int dbr_size;
    unsigned int reserved_size;
    unsigned int fat1_start_addr;
    unsigned int fat1_size;
    unsigned int fat2_start_addr;
    unsigned int fat2_size;
    unsigned int rootdir_start_addr;
    unsigned int rootdir_size;
    unsigned int data_start_addr;
} __attribute__((packed));

struct FAT_ROOT_DIR
{
    unsigned char DIR_Name[8];
    unsigned char DIR_Ext[3];
    unsigned char DIR_Attr;
    unsigned char reserved[10];
    unsigned short DIR_WriteTime;
    unsigned short DIR_WriteDate;
    unsigned short DIR_FstClus;
    unsigned int DIR_FileSize;
} __attribute__((packed));

struct ROOT_DIRS
{
    struct FAT_ROOT_DIR root_dir[CRITERIA_ROOT_DIR_NUM];
} __attribute__((packed));

//函数声明
// int write_image_file(FILE *infile_p, FILE *outfile_p, char *argv[]);
// int make_image_file(FILE *infile_p, char *argv[]);
// int view_dbr_sector_info(FILE *infile_p, char *argv[], struct FAT_BPB fat_bpb);
// int view_partition_info(FILE *infile_p, char *argv[], struct FAT_BPB fat_bpb, struct FAT_START_ADDR *fat_start_addr);
// int read_dbr_sector_info(FILE *infile_p, struct FAT_BPB *fat_bpb);
// void print_dbr_sector_info(FILE *infile_p, struct FAT_BPB fat_bpb);
// void print_partition_info(FILE *infile_p, struct FAT_BPB fat_bpb, struct FAT_START_ADDR *fat_start_addr);

/*
工具整体指令集：
    -h 帮助指令
    -o 输出文件指令
    -i 输入文件指令
    -s 起始扇区数指令
    -m 扇区总数指令
    -v 显示指令

工具界面注释规范：
    #################### 大标题
    ########## 中标题
*/
int main(int argc, char *argv[])
{
    //输入文件指针
    FILE *infile_p = NULL;
    //输出文件指针
    FILE *outfile_p = NULL;

    struct FAT_BPB fat_bpb = {0};
    struct FAT_START_ADDR fat_start_addr = {0};
    struct ROOT_DIRS root_dirs = {0};

    //程序开始
    if (argc > 6)
    {
        printf("fvdisk -h\n");
        return -1;
    }
    //帮助
    else if (argc == 2)
    {
        if (argv[1][0] == '-' && argv[1][1] == 'h' && argv[1][2] == '\0')
        {
            printf("#################### FVDISK TOOLS HELP ####################\n");
            printf("########## *GENERAL* Write File ##########\n");
            printf("fvdisk [.bin] -o [.vhd/.img] -s [start sector num]\n\n");
            printf("########## *GENERAL* Make Image File ##########\n");
            printf("fvdisk -i [.vhd/.img] -n [sector num]\n\n");
            printf("########## *FAT* Write File ##########\n");
            printf("fvdisk [.bin] -o [.vhd/.img] -f [partition num]\n\n");
            printf("########## *FAT* View Partition Info ##########\n");
            printf("fvdisk -i [.vhd/.img] -v [partition num]\n\n");
            printf("########## *FAT* View DBR Sector Info ##########\n");
            printf("fvdisk -i [.vhd/.img] -v DBR [partition num]\n\n");
        }
        else
        {
            printf("fvdisk -h\n");
            return -1;
        }
    }
    else if (argc == 6)
    {
        //*普通*写入文件
        //fvdisk [.bin] -o [.vhd/.img] -s [start sector num]
        if ((argv[2][0] == '-' && argv[2][1] == 'o' && argv[2][2] == '\0') && (argv[4][0] == '-' && argv[4][1] == 's' && argv[4][2] == '\0'))
        {
            // 测试输入文件
            infile_p = fopen(argv[1], "rb");
            // 测试输出文件
            outfile_p = fopen(argv[3], "rb+");
            if (infile_p == NULL)
            {
                printf("[ ERROR INFO ] The input file cannot open!\n[ FILE NAME ] %s\n", argv[1]);
                return -1;
            }
            if (outfile_p == NULL)
            {
                printf("[ ERROR INFO ] The output file cannot open!\n[ FILE NAME ] %s\n", argv[3]);
                return -1;
            }

            //普通写入文件
            return write_image_file(infile_p, outfile_p, argv);
        }
        //*FAT文件系统*写入文件
        //fvdisk [.bin] -o [.vhd/.img] -f [partition num]
        else if ((argv[2][0] == '-' && argv[2][1] == 'o' && argv[2][2] == '\0') && (argv[4][0] == '-' && argv[4][1] == 'f' && argv[4][2] == '\0'))
        {
            // 测试输入文件
            infile_p = fopen(argv[1], "rb");
            // 测试输出文件
            outfile_p = fopen(argv[3], "rb+");
            if (infile_p == NULL)
            {
                printf("[ ERROR INFO ] The input file cannot open!\n[ FILE NAME ] %s\n", argv[1]);
                return -1;
            }
            if (outfile_p == NULL)
            {
                printf("[ ERROR INFO ] The output file cannot open!\n[ FILE NAME ] %s\n", argv[3]);
                return -1;
            }

            //查看DBR扇区信息
            view_dbr_sector_info(outfile_p, argv, &fat_bpb);

            //读取显示分区信息
            view_partition_info(outfile_p, argv, fat_bpb, &fat_start_addr);
            
            // /* 测试专用 --- 清空根目录区 */
            // int zero = 0;
            // fseek(outfile_p, fat_start_addr.rootdir_start_addr, SEEK_SET);
            // fwrite(&zero, 1, sizeof(struct ROOT_DIRS), outfile_p);
            // fseek(outfile_p, fat_start_addr.fat1_start_addr, SEEK_SET);
            // fwrite(&zero, 1, fat_start_addr.fat1_size, outfile_p);
            // fseek(outfile_p, fat_start_addr.data_start_addr, SEEK_SET);
            // fwrite(&zero, 1, 512 * 1024, outfile_p);
            // return 0;
            // /* 测试专用 */

            //读取根目录区所有文件信息
            fseek(outfile_p, fat_start_addr.rootdir_start_addr, SEEK_SET);
            fread(&root_dirs, 1, sizeof(struct ROOT_DIRS), outfile_p);

            //获取文件名称地址
            char file_name_ptr[8];
            char file_ext_ptr[3];
            char file_attr[1];
            unsigned int file_size = 0;
            unsigned int file_all_clus = 0;
            unsigned int file_start_clus = 0;
            unsigned char *tmp_data_ptr;
            unsigned short *tmp_fat1_ptr = (unsigned short *)malloc(fat_start_addr.fat1_size);

            char *name_ptr = strrchr(argv[1], '/');
            char *ext_ptr = strrchr(name_ptr, '.');
            name_ptr += 1;
            ext_ptr += 1;

            if (ext_ptr != NULL)
            {
                memcpy(file_name_ptr, name_ptr, 8);
                memcpy(file_ext_ptr, ext_ptr, 3);

                // 对齐文件名称
                for (int i = strlen(name_ptr) - (strlen(ext_ptr) + 1); i < 8; i++)
                {
                    file_name_ptr[i] = ' ';
                }

                //对齐文件扩展名
                for (int i = 0; i < 3; i++)
                {
                    if (file_ext_ptr[i] == 0x00)
                    {
                        file_ext_ptr[i] = ' ';
                    }
                }

                //转大写处理
                strupr(file_name_ptr);
                strupr(file_ext_ptr);
            }
            else
            {
                memcpy(file_name_ptr, name_ptr, 8);

                // 对齐文件名称
                for (int i = strlen(name_ptr); i < 8; i++)
                {
                    file_name_ptr[i] = ' ';
                }

                //对齐文件扩展名
                memcpy(file_ext_ptr, "   ", 3);

                //转大写处理
                strupr(file_name_ptr);
            }
            
            //卷标
            if (file_ext_ptr[0] == ' ')
            {
                file_attr[0] = 0x08;
            }
            //存档
            else
            {
                file_attr[0] = 0x20;
            }

            //获取文件大小
            fseek(infile_p, 0, SEEK_END);
            file_size = ftell(infile_p);

            //获取起始簇号
            fseek(outfile_p, fat_start_addr.fat1_start_addr, SEEK_SET);
            fread(tmp_fat1_ptr, 1, fat_start_addr.fat1_size, outfile_p);
            for (int i = 2; i < (fat_start_addr.fat1_size / 2) - 2; i++)
            {
                if (tmp_fat1_ptr[i] == 0x0000)
                {
                    //存储文件起始簇号
                    file_start_clus = i;
                    printf("%d", i);
                    break;
                }
            }

            //计算每簇簇号
            int file_last_clus = 0;
            tmp_fat1_ptr[0] = 0xFFF8;
            tmp_fat1_ptr[1] = 0xFFFF;
            fseek(outfile_p, fat_start_addr.fat1_start_addr, SEEK_SET);
            fwrite(tmp_fat1_ptr, 1, fat_start_addr.fat1_size, outfile_p);
            
            file_all_clus = EASY_CEIL(file_size, (fat_bpb.BPB_BytesPerSec * fat_bpb.BPB_SecPerClus));
            //printf("%d\n", file_all_clus);

            if (((file_start_clus + file_all_clus) - 1) > 2)
            {
                int i;
                for (i = file_start_clus; i < (file_start_clus + file_all_clus) - 1; i++)
                {
                    if (tmp_fat1_ptr[i] == 0x0000)
                    {
                        //计算存储下一个簇号
                        tmp_fat1_ptr[i] = i + 1;

                        //写入FAT表中
                        fseek(outfile_p, fat_start_addr.fat1_start_addr, SEEK_SET);
                        fwrite(tmp_fat1_ptr, 1, fat_start_addr.fat1_size, outfile_p);
                    }
                }

                tmp_fat1_ptr[i] = 0xffff;
                file_last_clus = i;
            }
            else
            {
                tmp_fat1_ptr[file_start_clus] = 0xffff;
            }
            
            fseek(outfile_p, fat_start_addr.fat1_start_addr, SEEK_SET);
            fwrite(tmp_fat1_ptr, 1, fat_start_addr.fat1_size, outfile_p);

            //printf("\n%d %d %d %d %X\n", file_last_clus, file_start_clus, file_all_clus, file_size, tmp_data_ptr[0]);

            //获取文件数据
            fseek(infile_p, 0, SEEK_SET);
            tmp_data_ptr = (unsigned char *)malloc(file_size);
            fread(tmp_data_ptr, 1, file_size, infile_p);

            printf("\n%d\n", file_size);

            // return 0;

            //将文件数据写入到对应的簇号
            int start_addr = fat_start_addr.data_start_addr + ((file_start_clus - 2) * fat_bpb.BPB_SecPerClus * fat_bpb.BPB_BytesPerSec);
            fseek(outfile_p, start_addr, SEEK_SET);
            for (int i = 0; i < file_size; i++)
            {
                fwrite(&tmp_data_ptr[i], 1, 1, outfile_p);
            }
            
            printf("\n%X\n", start_addr);

            //寻找空目录项
            for (int i = 0; i < fat_start_addr.rootdir_size / 32; i++)
            {
                if (root_dirs.root_dir[i].DIR_Attr == 0x00)
                {
                    //写入文件名称

                    //写入文件扩展名
                    memcpy(&root_dirs.root_dir[i].DIR_Attr, file_attr, 1);
                    memcpy(root_dirs.root_dir[i].DIR_Name, file_name_ptr, 8);
                    memcpy(root_dirs.root_dir[i].DIR_Ext, file_ext_ptr, 3);
                    memcpy(&root_dirs.root_dir[i].DIR_FileSize, &file_size, 4);
                    memcpy(&root_dirs.root_dir[i].DIR_FstClus, &file_start_clus, 2);

                    //将新的根目录区结构体写入根目录区
                    fseek(outfile_p, fat_start_addr.rootdir_start_addr, SEEK_SET);
                    fwrite(&root_dirs, 1, sizeof(struct ROOT_DIRS), outfile_p);

                    /* 测试专用 */
                    printf("ok");
                    /* 测试专用 */
                    break;
                }
            }
            

            return 0;
        }
        //*FAT文件系统*查看DBR扇区信息
        //fvdisk -i [.vhd/.img] -v DBR [partition num]
        else if ((argv[1][0] == '-' && argv[1][1] == 'i' && argv[1][2] == '\0') && (argv[3][0] == '-' && argv[3][1] == 'v' && argv[3][2] == '\0') && (argv[4][0] == 'D' && argv[4][1] == 'B' && argv[4][2] == 'R' && argv[4][3] == '\0'))
        {
            // 测试输入文件
            infile_p = fopen(argv[2], "rb+");
            if (infile_p == NULL)
            {
                printf("[ ERROR INFO ] The input file cannot open!\n[ FILE NAME ] %s\n", argv[2]);
                return -1;
            }

            //查看DBR扇区信息
            view_dbr_sector_info(infile_p, argv, &fat_bpb);

            //输出BPB信息
            print_dbr_sector_info(infile_p, fat_bpb);

            return 0;
        }
        else
        {
            printf("[ ERROR INFO ] No output/input file or parameters!\n");
            printf("[ HELP INFO ] fvdisk -h\n");
            return -1;
        }
    }
    else if(argc == 5)
    {
        //*普通*创建镜像文件
        //fvdisk -i [.vhd/.img] -n [sector num]
        if ((argv[1][0] == '-' && argv[1][1] == 'i' && argv[1][2] == '\0') && (argv[3][0] == '-' && argv[3][1] == 'n' && argv[3][2] == '\0')) {
            // 测试输入文件
            infile_p = fopen(argv[2], "wb+");
            if (infile_p == NULL)
            {
                printf("[ ERROR INFO ] The input file cannot open!\n[ FILE NAME ] %s\n", argv[2]);
                return -1;
            }

            //创建镜像文件
            return make_image_file(infile_p, argv);
        }
        //*FAT文件系统*查看分区信息
        //fvdisk -i [.vhd/.img] -v [partition num]
        else if ((argv[1][0] == '-' && argv[1][1] == 'i' && argv[1][2] == '\0') && (argv[3][0] == '-' && argv[3][1] == 'v' && argv[3][2] == '\0'))
        {
            //测试输入文件
            infile_p = fopen(argv[2], "rb+");
            if(infile_p == NULL)
            {
                printf("[ ERROR INFO ] The input file cannot open!\n[ FILE NAME ] %s\n", argv[2]);
                return -1;
            }

            //读取显示分区信息
            view_partition_info(infile_p, argv, fat_bpb, &fat_start_addr);
            
            //输出分区信息
            print_partition_info(infile_p, fat_bpb, &fat_start_addr);

            //读取根目录区所有文件信息
            fseek(infile_p, fat_start_addr.rootdir_start_addr, SEEK_SET);
            fread(&root_dirs, 1, sizeof(struct ROOT_DIRS), infile_p);

            printf("#################### FILE INFO ####################\n");
            for (int i = 0; i < fat_start_addr.rootdir_size / 32; i++)
            {
                if ((root_dirs.root_dir[i].DIR_Name != 0 && root_dirs.root_dir[i].DIR_Ext != 0 && root_dirs.root_dir[i].DIR_FileSize != 0))
                {
                    printf("[ FILE ] %.8s   %.3s   %04d(0x%04X)\n", root_dirs.root_dir[i].DIR_Name, root_dirs.root_dir[i].DIR_Ext, root_dirs.root_dir[i].DIR_FileSize, root_dirs.root_dir[i].DIR_FileSize);
                }
            }
            
            return 0;
        }
        else
        {
            printf("[ ERROR INFO ] No output/input file or parameters!\n");
            printf("[ HELP INFO ] fvdisk -h\n");
            return -1;
        }
    }
    else
    {
        printf("[ ERROR INFO ] No output/input file or parameters!\n");
        printf("[ HELP INFO ] fvdisk -h\n");
        return -1;
    }
}

/*
@ infile_p  输入文件指针
@ argv      指令集
@ fat_bpb   FAT BPB结构体
@ fat_start_addr   FAT起始地址结构体指针
*/
int view_partition_info(FILE *infile_p, char *argv[], struct FAT_BPB fat_bpb, struct FAT_START_ADDR *fat_start_addr)
{
    long long file_buf = 0;
    int file_bytes_size = 0;
    int dbr_start_addr = 0;

    //寻找DBR起始地址(判断标准跳转指令：0xEB3C90)
    fseek(infile_p, dbr_start_addr, SEEK_SET);
    fread(fat_bpb.Jmp_short, 1, 3, infile_p);
    while (((fat_bpb.Jmp_short[0] << 16) | (fat_bpb.Jmp_short[1] << 8) | fat_bpb.Jmp_short[2]) != 0xEB3C90)
    {
        dbr_start_addr += 512;
        fseek(infile_p, dbr_start_addr, SEEK_SET);
        fread(fat_bpb.Jmp_short, 1, 3, infile_p);
    }

    //读取剩余BPB信息
    read_dbr_sector_info(infile_p, &fat_bpb);

    //计算FAT文件系统的起始地址和大小
    //DBR扇区
    unsigned int dbr_size = fat_bpb.BPB_BytesPerSec;
    fat_start_addr->dbr_size = dbr_size;
    fat_start_addr->dbr_start_addr = dbr_start_addr;

    //保留扇区
    unsigned int reserved_size = fat_bpb.BPB_RsvdSecCnt * fat_bpb.BPB_BytesPerSec;
    fat_start_addr->reserved_size = reserved_size;

    //FAT表1
    unsigned int fat1_size = fat_bpb.BPB_FatSz16 * fat_bpb.BPB_BytesPerSec;
    unsigned int fat1_start_addr = dbr_start_addr + reserved_size;
    fat_start_addr->fat1_size = fat1_size;
    fat_start_addr->fat1_start_addr = fat1_start_addr;

    //FAT表2
    unsigned int fat2_size = fat_bpb.BPB_FatSz16 * fat_bpb.BPB_BytesPerSec;
    unsigned int fat2_start_addr = fat1_start_addr + fat1_size;
    fat_start_addr->fat2_size = fat2_size;
    fat_start_addr->fat2_start_addr = fat2_start_addr;

    //根目录区
    unsigned int rootdir_size = fat_bpb.BPB_RootEntCnt * 32;
    unsigned int rootdir_start_addr = fat2_start_addr + fat2_size;
    fat_start_addr->rootdir_size = rootdir_size;
    fat_start_addr->rootdir_start_addr = rootdir_start_addr;

    //数据区
    unsigned int data_start_addr = rootdir_start_addr + rootdir_size;
    fat_start_addr->data_start_addr = data_start_addr;

    return 0;
}

/*
@ infile_p  输入文件指针
@ argv      指令集
@ fat_bpb   FAT BPB结构体
*/
int view_dbr_sector_info(FILE *infile_p, char *argv[], struct FAT_BPB *fat_bpb)
{
    long long file_buf = 0;
    int file_bytes_size = 0;
    int dbr_start_addr = 0;

    //寻找DBR起始地址(判断标准跳转指令：0xEB3C90)
    fseek(infile_p, dbr_start_addr, SEEK_SET);
    fread(fat_bpb->Jmp_short, 1, 3, infile_p);
    while (((fat_bpb->Jmp_short[0] << 16) | (fat_bpb->Jmp_short[1] << 8) | fat_bpb->Jmp_short[2]) != 0xEB3C90)
    {
        dbr_start_addr += 512;
        fseek(infile_p, dbr_start_addr, SEEK_SET);
        fread(fat_bpb->Jmp_short, 1, 3, infile_p);
    }

    //读取剩余BPB信息
    read_dbr_sector_info(infile_p, fat_bpb);

    return 0;
}

/*
@ infile_p  输入文件指针
@ fat_bpb   FAT BPB结构体指针
*/
int read_dbr_sector_info(FILE *infile_p, struct FAT_BPB *fat_bpb)
{
    if (fat_bpb == NULL) {
        return -1;
    }

    fread(fat_bpb->BPB_OEMName, 1, 8, infile_p);
    fread(&fat_bpb->BPB_BytesPerSec, 1, 2, infile_p);
    fread(&fat_bpb->BPB_SecPerClus, 1, 1, infile_p);
    fread(&fat_bpb->BPB_RsvdSecCnt, 1, 2, infile_p);
    fread(&fat_bpb->BPB_NumFATs, 1, 1, infile_p);
    fread(&fat_bpb->BPB_RootEntCnt, 1, 2, infile_p);
    fread(&fat_bpb->BPB_TotSec16, 1, 2, infile_p);
    fread(&fat_bpb->BPB_Media, 1, 1, infile_p);
    fread(&fat_bpb->BPB_FatSz16, 1, 2, infile_p);
    fread(&fat_bpb->BPB_SecPerTrk, 1, 2, infile_p);
    fread(&fat_bpb->BPB_NumHeads, 1, 2, infile_p);
    fread(&fat_bpb->BPB_HiddSec, 1, 4, infile_p);
    fread(&fat_bpb->BPB_TotSec32, 1, 4, infile_p);
    fread(&fat_bpb->BS_drvNum, 1, 1, infile_p);
    fread(&fat_bpb->BS_Reserved1, 1, 1, infile_p);
    fread(&fat_bpb->BS_BootSig, 1, 1, infile_p);
    fread(&fat_bpb->BS_VolID, 1, 4, infile_p);
    fread(fat_bpb->BS_VolLab, 1, 11, infile_p);
    fread(fat_bpb->BS_FilSysType, 1, 8, infile_p);

    return 0;
}

/*
@ infile_p  输入文件指针
@ fat_bpb   FAT BPB结构体
@ fat_start_addr FAT起始地址结构体
*/
void print_partition_info(FILE *infile_p, struct FAT_BPB fat_bpb, struct FAT_START_ADDR *fat_start_addr)
{
    unsigned int dbr_size = fat_start_addr->dbr_size;
    unsigned int dbr_start_addr = fat_start_addr->dbr_start_addr;
    unsigned int fat1_size = fat_start_addr->fat1_size;
    unsigned int fat1_start_addr = fat_start_addr->fat1_start_addr;
    unsigned int fat2_size = fat_start_addr->fat2_size;
    unsigned int fat2_start_addr = fat_start_addr->fat2_start_addr;
    unsigned int rootdir_size = fat_start_addr->rootdir_size;
    unsigned int rootdir_start_addr = fat_start_addr->rootdir_start_addr;
    unsigned int data_start_addr = fat_start_addr->data_start_addr;

    //输出分区信息
    printf("#################### PARTITION INFO ####################\n");
    printf("########### DBR INFO ###########\n");
    printf("[ DBR SIZE ]        %06d   (0x%X)\n", dbr_size, dbr_size);
    printf("[ START SEC NUM ]   %d\n", dbr_start_addr / 512);
    printf("[ START ADDR ]      0x%X\n\n", dbr_start_addr);

    printf("########### FAT1 INFO ###########\n");
    printf("[ FAT1 SIZE ]       %06d   (0x%X)\n", fat1_size, fat1_size);
    printf("[ START SEC NUM ]   %d\n", fat1_start_addr / 512);
    printf("[ START ADDR ]      %X\n\n", fat1_start_addr, fat1_start_addr);

    printf("########### FAT2 INFO ###########\n");
    printf("[ FAT2 SIZE ]       %06d   (0x%X)\n", fat2_size, fat2_size);
    printf("[ START SEC NUM ]   %d\n", fat2_start_addr / 512);
    printf("[ START ADDR ]      %X\n\n", fat2_start_addr, fat2_start_addr);

    printf("########### ROOTDIR INFO ###########\n");
    printf("[ ROOTDIR SIZE ]    %06d   (0x%X)\n", rootdir_size, rootdir_size);
    printf("[ START SEC NUM ]   %d\n", rootdir_start_addr / 512);
    printf("[ START ADDR ]      %X\n\n", rootdir_start_addr, rootdir_start_addr);

    printf("########### DATA INFO ###########\n");
    printf("[ START SEC NUM ]   %d\n", data_start_addr / 512);
    printf("[ START ADDR ]      %X\n\n", data_start_addr, data_start_addr);

    return;
}

/*
@ infile_p  输入文件指针
@ fat_bpb   FAT BPB结构体
*/
void print_dbr_sector_info(FILE *infile_p, struct FAT_BPB fat_bpb)
{
    printf("#################### DBR SECTOR INFO ####################\n");
    printf("[ BPB JMP SHORT ]       0x%X%X%X\n", fat_bpb.Jmp_short[0], fat_bpb.Jmp_short[1], fat_bpb.Jmp_short[2]);
    printf("[ BPB OEM NAME ]        %.8s\n", fat_bpb.BPB_OEMName);
    printf("[ BPB BYTES PER SEC ]   %06d      (0x%X)\n", fat_bpb.BPB_BytesPerSec, fat_bpb.BPB_BytesPerSec);
    printf("[ BPB SEC PER CLUS ]    %06d      (0x%X)\n", fat_bpb.BPB_SecPerClus, fat_bpb.BPB_SecPerClus);
    printf("[ BPB RSVD SEC CNT ]    %06d      (0x%X)\n", fat_bpb.BPB_RsvdSecCnt, fat_bpb.BPB_RsvdSecCnt);
    printf("[ BPB NUM FATS ]        %06d      (0x%X)\n", fat_bpb.BPB_NumFATs, fat_bpb.BPB_NumFATs);
    printf("[ BPB ROOT ENT CNT ]    %06d      (0x%X)\n", fat_bpb.BPB_RootEntCnt, fat_bpb.BPB_RootEntCnt);
    printf("[ BPB TOT SEC 16 ]      %06d      (0x%X)\n", fat_bpb.BPB_TotSec16, fat_bpb.BPB_TotSec16);
    printf("[ BPB MEDIA ]           %06d      (0x%X)\n", fat_bpb.BPB_Media, fat_bpb.BPB_Media);
    printf("[ FAT SZ 16 ]           %06d      (0x%X)\n", fat_bpb.BPB_FatSz16, fat_bpb.BPB_FatSz16);
    printf("[ BPB SEC PER TRK ]     %06d      (0x%X)\n", fat_bpb.BPB_SecPerTrk, fat_bpb.BPB_SecPerTrk);
    printf("[ BPB NUM HEADS ]       %06d      (0x%X)\n", fat_bpb.BPB_NumHeads, fat_bpb.BPB_NumHeads);
    printf("[ BPB HIDDEN SEC ]      %06d      (0x%X)\n", fat_bpb.BPB_HiddSec, fat_bpb.BPB_HiddSec);
    printf("[ BPB TOT SEC 32 ]      %06d      (0x%X)\n", fat_bpb.BPB_TotSec32, fat_bpb.BPB_TotSec32);
    printf("[ BS DRV NUM ]          %06d      (0x%X)\n", fat_bpb.BS_drvNum, fat_bpb.BS_drvNum);
    printf("[ BS RESERVED1 ]        %06d      (0x%X)\n", fat_bpb.BS_Reserved1, fat_bpb.BS_Reserved1);
    printf("[ BS BOOTSIG ]          %06d      (0x%X)\n", fat_bpb.BS_BootSig, fat_bpb.BS_BootSig);
    printf("[ BS VOLID ]            %010d  (0x%X)\n", fat_bpb.BS_VolID, fat_bpb.BS_VolID);
    
    printf("[ BS VOL LABEL ]        ");
    for (int i = 0; i < 11; ++i) {
        printf("%c", fat_bpb.BS_VolLab[i]);
    }
    printf("\n");

    printf("[ BS FIL SYS TYPE ]     ");
    for (int i = 0; i < 8; ++i) {
        printf("%c", fat_bpb.BS_FilSysType[i]);
    }
    printf("\n\n");

    return;
}

/*
@ infile_p  输入文件指针
@ argv      指令集
*/
int make_image_file(FILE *infile_p, char *argv[])
{
    long long file_buf = 0;
    int file_bytes_size = 0;

    //计算总扇区数
    int sec_num = strtol(argv[4], 0, 0);
    sec_num *= CRITERIA_SECTOR_NUM;
    int cnt_bytes_size = sec_num;
    fseek(infile_p, 0, SEEK_SET);


    //写入数据
    while(cnt_bytes_size > 0)
    {
        if(fwrite(&file_buf, 1, 1, infile_p) != 0)
        {
            file_bytes_size++;
        }

        cnt_bytes_size--;
    }


    //输出信息
    printf("[ SIZE INFO ] %d(0x%X) bytes   %d sector\n", file_bytes_size, file_bytes_size, file_bytes_size / CRITERIA_SECTOR_NUM);
    
    fclose(infile_p);

    return 0;
}

/*
@ infile_p  输入文件指针
@ outfile_p 输出文件指针
@ argv      指令集
*/
int write_image_file(FILE *infile_p, FILE *outfile_p, char *argv[])
{
    int file_bytes_size = 0;
    long long file_buf = 0;

    //计算起始扇区数
    int start_sec_num = strtol(argv[5], 0, 0);
    start_sec_num *= CRITERIA_SECTOR_NUM;
    fseek(outfile_p, start_sec_num, SEEK_SET);


    //写入数据
    while(fread(&file_buf, 1, 1, infile_p) != 0)
    {
        if(fwrite(&file_buf, 1, 1, outfile_p) != 0)
        {
            file_bytes_size++;
        }
    }


    //输出信息
    printf("[ SIZE INFO ] %d(0x%X) bytes   %d sector\n", file_bytes_size, file_bytes_size, file_bytes_size / CRITERIA_SECTOR_NUM);
    
    fclose(infile_p);
    fclose(outfile_p);

    return 0;
}

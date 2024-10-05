/*
fat16.c
FAT16文件系统实现
Copyright W24 Studio 
*/

#include <hd.h>
#include <mm.h>
#include <fat16.h>
#include <cmos.h>
#include <string.h>

static file_t file_table[MAX_FILE_NUM];

// 格式化文件系统
int fat16_format_hd()
{
	 static unsigned char default_boot_code[] = {
		0xB8, 0xC0, 0x07, 0x8E, 0xC8, 0x8E, 0xD8, 0x8E, 0xD0, 0xBE, 0x1F, 0x00, 0xE8, 0x02, 0x00, 0xEB, 
		0xFE, 0x8A, 0x04, 0x3C, 0x00, 0x74, 0x07, 0xB4, 0x0E, 0xCD, 0x10, 0x46, 0xEB, 0xF3, 0xC3, 0x54, 
		0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x6E, 0x6F, 0x74, 0x20, 0x61, 0x20, 0x76, 0x61, 0x6C, 
		0x69, 0x64, 0x20, 0x62, 0x6F, 0x6F, 0x74, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x2E, 0x00
	};// 这段代码的意思是：输出一段信息，是用nasm写完编译的
    char *fat1 = (char *) malloc(512);
    hd_read(FAT1_START_LBA, 1, fat1); // 读取FAT表第一个扇区
    if (fat1[0] == 0xff) { // 如果第一个字节是0xff，那就是有文件系统
        free(fat1);
        return 1; // 那就没有必要格式化了
    }
    free(fat1);
    int sectors = get_hd_sects(); // 获取硬盘扇区大小先存着
    bpb_hdr_t hdr; // 构造一个引导扇区
    hdr.BS_jmpBoot[0] = 0xeb;
    hdr.BS_jmpBoot[1] = 0x3c; // jmp到default_boot_code
    hdr.BS_jmpBoot[2] = 0x90; // nop凑够3字节
    strcpy(hdr.BS_OEMName, "NEUMANN "); // OEM为neumann
    hdr.BPB_BytsPerSec = 512;
    hdr.BPB_SecPerClust = 1;
    hdr.BPB_RsvdSecCnt = 1;
    hdr.BPB_NumFATs = 2; // 总共两个FAT，这是规定
    hdr.BPB_RootEntCnt = 512; // 根目录区32个扇区，一个目录项占32字节，32*512/32=512
    if (sectors < (1 << 16) - 1) {
        hdr.BPB_TotSec16 = sectors;
        hdr.BPB_TotSec32 = 0;
    } else {
        hdr.BPB_TotSec16 = 0;
        hdr.BPB_TotSec32 = sectors;
    }
    hdr.BPB_Media = 0xf8; // 硬盘统一数据
    hdr.BPB_FATSz16 = 32; // FAT16是这样的
    hdr.BPB_SecPerTrk = 63; // 硬盘统一数据
    hdr.BPB_NumHeads = 16; // 硬盘统一数据
    hdr.BPB_HiddSec = 0;
    hdr.BS_DrvNum = 0x80; // 硬盘统一数据
    hdr.BS_Reserved1 = 0;
    hdr.BS_BootSig = 0x29;
    hdr.BS_VolID = 0;
    strcpy(hdr.BS_VolLab, "NEUMANNBOOT"); // 可以随便改
    strcpy(hdr.BS_FileSysType, "FAT16   "); // 尽量别改
    memset(hdr.BS_BootCode, 0, 448);
    memcpy(hdr.BS_BootCode, default_boot_code, sizeof(default_boot_code));
    hdr.BS_BootEndSig = 0xaa55;
    hd_write(0, 1, &hdr); // 引导扇区就这样了
    char initial_fat[512] = {0xff, 0xf8, 0xff, 0xff, 0}; // 硬盘统一数据
    hd_write(FAT1_START_LBA, 1, &initial_fat); // 写入FAT1
    hd_write(FAT1_START_LBA + FAT1_SECTORS, 1, &initial_fat); // 写入FAT2
    return 0;
}

// 把原文件名改编为FAT16所要求的8.3格式
static int lfn2sfn(const char *lfn, char *sfn)
{
    int len = strlen(lfn), last_dot = -1;
    for (int i = len - 1; i >= 0; i--) { // 从尾到头遍历，寻找最后一个.的位置
        if (lfn[i] == '.') { // 找到了
            last_dot = i; // 最后一个.赋值一下
            break; // 跳出循环
        }
    }
    
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
    for (int i = len_ext; i < 3; i++) sfn[i + 8] = ' '; // 用空格填充剩余部分
    sfn[11] = 0; // 文件名的结尾加一个\0
    return 0; // 正常退出
}

// 读取根目录目录项
fileinfo_t *read_dir_entries(int *dir_ents)
{
    fileinfo_t *root_dir = (fileinfo_t *) malloc(ROOT_DIR_SECTORS * SECTOR_SIZE);
    hd_read(ROOT_DIR_START_LBA, ROOT_DIR_SECTORS, root_dir); // 将根目录的所有扇区全部读入
    int i;
    for (i = 0; i < MAX_FILE_NUM; i++) {
        if (root_dir[i].name[0] == 0) break; // 如果名字的第一个字节是0，那就说明这里没有文件
    }
    *dir_ents = i; // 将目录项个数写到指针里
    return root_dir; // 返回根目录
}

// 创建文件
int fat16_create_file(fileinfo_t *finfo, char *filename)
{
    if (filename[0] == 0xe5) filename[0] = 0x05; // 如上，若第一个字节为 0xe5，需要更换为 0x05
    char sfn[20] = {0};
    int ret = lfn2sfn(filename, sfn); // 将文件名转换为8.3文件名
    if (ret) return -1; // 文件名不符合8.3规范，返回
    int entries;
    fileinfo_t *root_dir = read_dir_entries(&entries); // 读取所有根目录项
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
    if (free_slot == MAX_FILE_NUM) { // 如果空闲位置已经到达根目录末尾
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
    current_time_t ctime;
    get_current_time(&ctime); // 获取当前时间
    // 按照前文所说依次填入date和time
    root_dir[free_slot].date = ((ctime.year - 1980) << 9) | (ctime.month << 5) | ctime.day;
    root_dir[free_slot].time = (ctime.hour << 11) | (ctime.min << 5) | ctime.sec;
    if (finfo) *finfo = root_dir[free_slot]; // 创建完了不能不管，传给finfo留着
    hd_write(ROOT_DIR_START_LBA, ROOT_DIR_SECTORS, root_dir); // 将新的根目录区写回硬盘
    free(root_dir); // 成功完成
    return 0;
}

// 打开文件
int fat16_open_file(fileinfo_t *finfo, char *filename)
{
    char sfn[20] = {0};
    int ret = lfn2sfn(filename, sfn); // 将原文件名转换为8.3
    if (ret) return -1; // 转换失败，不用打开了
    int entries;
    fileinfo_t *root_dir = read_dir_entries(&entries); // 读取所有目录项
    int file_index = entries; // filename对应文件的索引
    for (int i = 0; i < entries; i++) {
        if (!memcmp(root_dir[i].name, sfn, 8) && !memcmp(root_dir[i].ext, sfn + 8, 3)) {
            file_index = i; // 找到了
            break;
        }
    }
    if (file_index < entries) { // 如果找到了……
        *finfo = root_dir[file_index]; // 那么把对应的文件存到finfo里
        free(root_dir);
        return 0;
    }
    else {
        finfo = NULL; // 这一句实际上是没有用的
        free(root_dir);
        return -1;
    }
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

// 读取文件，当然要有素质地一次读整个文件啦
int fat16_read_file(fileinfo_t *finfo, void *buf)
{
    uint16_t clustno = finfo->clustno; // finfo中记录的第一个簇号
    char *clust = (char *) malloc(512); // 单独给簇分配一个缓冲区，直接往buf里写也行
    do {
        read_nth_clust(clustno, clust); // 将该簇号对应的簇读取进来
        memcpy(buf, clust, 512); // 拷贝入buf
        buf += 512; // buf后推一个扇区
        clustno = get_nth_fat(clustno); // 获取下一个簇号
        if (clustno >= 0xFFF8) break; // 文件结束，退出循环
    } while (1);
    free(clust); // 读完了，释放临时缓冲区
    return 0; // 返回
}

// 删除文件
int fat16_delete_file(char *filename) // 什么？为什么不传finfo？删除一个已经打开的文件，听上去很别扭不是吗（虽然在Linux下这很正常）
{
    char sfn[20] = {0};
    int ret = lfn2sfn(filename, sfn); // 将文件名转换为8.3文件名
    if (ret) return -1;
    int entries;
    fileinfo_t *root_dir = read_dir_entries(&entries); // 读取根目录
    int file_ind = -1;
    for (int i = 0; i < entries; i++) {
        if (!memcmp(root_dir[i].name, sfn, 8) && !memcmp(root_dir[i].ext, sfn + 8, 3)) {
            file_ind = i; // 找到对应文件了
            break;
        }
    }
    if (file_ind == -1) { // 没有找到
        free(root_dir); // 不用删了
        return -1;
    }
    root_dir[file_ind].name[0] = 0xe5; // 标记为已删除
    hd_write(ROOT_DIR_START_LBA, ROOT_DIR_SECTORS, root_dir); // 更新根目录区数据
    free(root_dir); // 释放临时缓冲区
    if (root_dir[file_ind].clustno == 0) {
        return 0; // 内容空空，那就到这里就可以了
    }
    unsigned short clustno = root_dir[file_ind].clustno, next_clustno; // 开始清理文件所占有的簇
    while (1) {
        next_clustno = get_nth_fat(clustno); // 找到这个文件下一个簇的簇号
        set_nth_fat(clustno, 0); // 把下一个簇的簇号设为0，这样就找不到下一个簇了
        if (next_clustno >= 0xfff8) break; // 已经删完了，直接返回
        clustno = next_clustno; // 下一个簇设为当前簇
    }
    return 0; // 删除完成
}

// 写入文件，为简单起见相当于覆盖了
int fat16_write_file(fileinfo_t *finfo, const void *buf, uint32_t size)
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
    // 最后修改一下文件属性
    current_time_t ctime;
    get_current_time(&ctime); // 获取当前日期
    // 更新日期和时间
    finfo->date = ((ctime.year - 1980) << 9) | (ctime.month << 5) | ctime.day;
    finfo->time = (ctime.hour << 11) | (ctime.min << 5) | ctime.sec;
    int entries;
    fileinfo_t *root_dir = read_dir_entries(&entries);
    for (int i = 0; i < entries; i++) {
        if (!memcmp(root_dir[i].name, finfo->name, 8) && !memcmp(root_dir[i].ext, finfo->ext, 3)) {
            root_dir[i] = *finfo; // 找到对应的文件，写进根目录
            break;
        }
    }
    hd_write(ROOT_DIR_START_LBA, ROOT_DIR_SECTORS, root_dir); // 同步到硬盘
    free(root_dir);
    return 0;
}

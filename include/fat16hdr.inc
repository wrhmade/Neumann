    BS_OEMName     db 'NEUMANN '    ; 固定的8个字节
    BPB_BytsPerSec dw 512           ; 每扇区固定512个字节
    BPB_SecPerClus db 1             ; 每簇固定1个扇区
    BPB_RsvdSecCnt dw 1             ; MBR固定占用1个扇区
    BPB_NumFATs    db 2             ; 我们实现的FAT16文件系统有2个FAT表
    BPB_RootEntCnt dw 512           ; 根目录区32个扇区，一个目录项32字节，共计32*512/32=512个目录项
    BPB_TotSec16   dw 0             ; 80MB硬盘的大小过大，不足以放到TotSec16
    BPB_Media      db 0xF8          ; 介质描述符，硬盘为0xF8
    BPB_FATSz16    dw 32            ; 一个FAT表所占的扇区数，FAT16 文件系统固定为32个扇区
    BPB_SecPerTrk  dw 63            ; 每磁道扇区数，80MB硬盘为63
    BPB_NumHeads   dw 16            ; 磁头数，bximage 的输出告诉我们是16个
    BPB_HiddSec    dd 0             ; 隐藏扇区数，没有
    BPB_TotSec32   dd 41943040      ; 若之前的 BPB_TotSec16 处没有记录扇区数，则由此记录，如果记录了，这里直接置0即可
    BS_DrvNum      db 0x80          ; int 13h 调用时所读取的驱动器号，由于挂载的是硬盘所以0x80 
    BS_Reserved1   db 0             ; 未使用，预留
    BS_BootSig     db 29h           ; 扩展引导标记
    BS_VolID       dd 0             ; 卷序列号，由于只挂载一个盘所以为0
    BS_VolLab      db 'NEUMANN    ' ; 卷标，11个字节
    BS_FileSysType db 'FAT16   '    ; 由于是 FAT16 文件系统，所以写入 FAT16 后补齐8个字节

FATSz                   equ 32      ; BPB_FATSz16
RootDirSectors          equ 32      ; 根目录大小
SectorNoOfRootDirectory equ 65      ; 根目录起始扇区
SectorNoOfFAT1          equ 1       ; 第一个FAT表的开始扇区
DeltaSectorNo           equ 63      ; 由于第一个簇不用，所以RootDirSectors要-2再加上根目录区首扇区和偏移才能得到真正的地址，故把RootDirSectors-2封装成一个常量

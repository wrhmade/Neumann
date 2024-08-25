    org 07c00h ; 告诉编译器程序将装载至0x7c00处
 
BaseOfStack             equ 07c00h ; 栈的基址

    jmp short LABEL_START
    nop ; BS_JMPBoot 由于要三个字节而jmp到LABEL_START只有两个字节 所以加一个nop
 
%include "fat16hdr.inc" ; 没错它会db一遍
%include "load.inc" ; 代替之前的常量

LABEL_START:
    mov ax, cs
    mov ds, ax
    mov es, ax ; 将ds es设置为cs的值（因为此时字符串和变量等存在代码段内）
    mov ss, ax ; 将堆栈段也初始化至cs
    mov sp, BaseOfStack ; 设置栈顶
 
    mov ax, 0600h ; AH=06h：向上滚屏，AL=00h：清空窗口
    mov bx, 0700h ; 空白区域缺省属性
    mov cx, 0 ; 左上：(0, 0)
    mov dx, 0184fh ; 右下：(80, 25)
    int 10h ; 执行
 
    mov dh, 0
    call DispStr ; Booting
 
    xor ah, ah ; 复位
    xor dl, dl
    int 13h ; 执行软驱复位
 
    mov word [wSectorNo], SectorNoOfRootDirectory ; 开始查找，将当前读到的扇区数记为根目录区的开始扇区（19）
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
    cmp word [wRootDirSizeForLoop], 0 ; 将剩余的根目录区扇区数与0比较
    jz LABEL_NO_LOADERBIN ; 相等，不存在Loader，进行善后
    dec word [wRootDirSizeForLoop] ; 减去一个扇区
    mov ax, BaseOfLoader
    mov es, ax
    mov bx, OffsetOfLoader ; 将es:bx设置为BaseOfLoader:OffsetOfLoader，暂且使用Loader所占的内存空间存放根目录区
    mov ax, [wSectorNo] ; 起始扇区：当前读到的扇区数（废话）
    mov cl, 1 ; 读取一个扇区
    call ReadSector ; 读入
 
    mov si, LoaderFileName ; 为比对做准备，此处是将ds:si设为Loader文件名
    mov di, OffsetOfLoader ; 为比对做准备，此处是将es:di设为Loader偏移量（即根目录区中的首个文件块）
    cld ; FLAGS.DF=0，即执行lodsb/lodsw/lodsd后，si自动增加
    mov dx, 10h ; 共16个文件块（代表一个扇区，因为一个文件块32字节，16个文件块正好一个扇区）
LABEL_SEARCH_FOR_LOADERBIN:
    cmp dx, 0 ; 将dx与0比较
    jz LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR ; 继续前进一个扇区
    dec dx ; 否则将dx减1
    mov cx, 11 ; 文件名共11字节
LABEL_CMP_FILENAME: ; 比对文件名
    cmp cx, 0 ; 将cx与0比较
    jz LABEL_FILENAME_FOUND ; 若相等，说明文件名完全一致，表示找到，进行找到后的处理
    dec cx ; cx减1，表示读取1个字符
    lodsb ; 将ds:si的内容置入al，si加1
    cmp al, byte [es:di] ; 此字符与LOADER  BIN中的当前字符相等吗？
    jz LABEL_GO_ON ; 下一个文件名字符
    jmp LABEL_DIFFERENT ; 下一个文件块
LABEL_GO_ON:
    inc di ; di加1，即下一个字符
    jmp LABEL_CMP_FILENAME ; 继续比较

LABEL_DIFFERENT:
    and di, 0FFE0h ; 指向该文件块开头
    add di, 20h ; 跳过32字节，即指向下一个文件块开头
    mov si, LoaderFileName ; 重置ds:si
    jmp LABEL_SEARCH_FOR_LOADERBIN ; 由于要重新设置一些东西，所以回到查找Loader循环的开头

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
    add word [wSectorNo], 1 ; 下一个扇区
    jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN ; 重新执行主循环

LABEL_NO_LOADERBIN: ; 若找不到loader.bin则到这里
    mov dh, 2
    call DispStr; 显示No LOADER
    jmp $

LABEL_FILENAME_FOUND:
    mov ax, RootDirSectors ; 将ax置为根目录首扇区（19）
    and di, 0FFE0h ; 将di设置到此文件块开头
    add di, 01Ah ; 此时的di指向Loader的FAT号
    mov cx, word [es:di] ; 获得该扇区的FAT号
    push cx ; 将FAT号暂存
    add cx, ax ; +根目录首扇区
    add cx, DeltaSectorNo ; 获得真正的地址
    mov ax, BaseOfLoader
    mov es, ax
    mov bx, OffsetOfLoader ; es:bx：读取扇区的缓冲区地址
    mov ax, cx ; ax：起始扇区号

LABEL_GOON_LOADING_FILE: ; 加载文件
    push ax
    push bx
    mov ah, 0Eh ; AH=0Eh：显示单个字符
    mov al, '.' ; AL：字符内容
    mov bl, 0Fh ; BL：显示属性
; 还有BH：页码，此处不管
    int 10h ; 显示此字符
    pop bx
    pop ax ; 上面几行的整体作用：在屏幕上打印一个点
 
    mov cl, 1
    call ReadSector ; 读取Loader第一个扇区
    pop ax ; 加载FAT号
    call GetFATEntry ; 加载FAT项
    cmp ax, 0FFFFh
    jz LABEL_FILE_LOADED ; 若此项=0FFF，代表文件结束，直接跳入Loader
    push ax ; 重新存储FAT号，但此时的FAT号已经是下一个FAT了
    mov dx, RootDirSectors
    add ax, dx ; +根目录首扇区
    add ax, DeltaSectorNo ; 获取真实地址
    add bx, [BPB_BytsPerSec] ; 将bx指向下一个扇区开头
    jmp LABEL_GOON_LOADING_FILE ; 加载下一个扇区

LABEL_FILE_LOADED:
    mov dh, 1 ; 打印第 1 条消息（Ready.）
    call DispStr
    jmp BaseOfLoader:OffsetOfLoader ; 跳入Loader！
 
wRootDirSizeForLoop dw RootDirSectors ; 查找loader的循环中将会用到
wSectorNo           dw 0              ; 用于保存当前扇区数
bOdd                db 0              ; 这个其实是下一节的东西，不过先放在这也不是不行
 
LoaderFileName      db "LOADER  BIN", 0 ; loader的文件名
 
MessageLength       equ 9 ; 下面是三条小消息，此变量用于保存其长度，事实上在内存中它们的排序类似于二维数组
BootMessage:        db "Booting  " ; 此处定义之后就可以删除原先定义的BootMessage字符串了
Message1            db "Ready.   " ; 显示已准备好
Message2            db "No LOADER" ; 显示没有Loader

DispStr:
    mov ax, MessageLength
    mul dh ; 将ax乘以dh后，结果仍置入ax（事实上远比此复杂，此处先解释到这里）
    add ax, BootMessage ; 找到给定的消息
    mov bp, ax ; 先给定偏移
    mov ax, ds
    mov es, ax ; 以防万一，重新设置es
    mov cx, MessageLength ; 字符串长度
    mov ax, 01301h ; ah=13h, 显示字符的同时光标移位
    mov bx, 0007h ; 黑底白字
    mov dl, 0 ; 第0行，前面指定的dh不变，所以给定第几条消息就打印到第几行
    int 10h ; 显示字符
    ret

ReadSector: ; 读硬盘扇区
; 从第eax号扇区开始，读取cl个扇区至es:bx
    push esi
    push di
    push es
    push bx
    mov esi, eax
    mov di, cx ; 备份ax,cx

; 读硬盘 第一步：设置要读取扇区数
    mov dx, 0x1f2
    mov al, cl
    out dx, al

    mov eax, esi ; 恢复ax

; 第二步：写入扇区号
    mov dx, 0x1f3
    out dx, al ; LBA 7~0位，写入0x1f3

    mov cl, 8
    shr eax, cl ; LBA 15~8位，写入0x1f4
    mov dx, 0x1f4
    out dx, al

    shr eax, cl
    mov dx, 0x1f5
    out dx, al ; LBA 23~16位，写入0x1f5

    shr eax, cl
    and al, 0x0f ; LBA 27~24位
    or al, 0xe0 ; 表示当前硬盘
    mov dx, 0x1f6 ; 写入0x1f6
    out dx, al

; 第三步：0x1f7写入0x20，表示读
    mov dx, 0x1f7 
    mov al, 0x20
    out dx, al

; 第四步：检测硬盘状态
.not_ready:
    nop
    in al, dx ; 读入硬盘状态
    and al, 0x88 ; 分离第4位，第7位
    cmp al, 0x08 ; 硬盘不忙且已准备好
    jnz .not_ready ; 不满足，继续等待

; 第五步：将数据从0x1f0端口读出
    mov ax, di ; di为要读扇区数，共需读di * 512 / 2次
    mov dx, 256
    mul dx
    mov cx, ax
    
    mov dx, 0x1f0
.go_on_read:
    in ax, dx
    mov [es:bx], ax
    add bx, 2
    loop .go_on_read
; 结束
    pop bx
    pop es
    pop di
    pop esi
    ret

GetFATEntry: ; 返回第ax个簇的值
    push es
    push bx
    push ax ; 都会用到，push一下
    mov ax, BaseOfLoader
    sub ax, 0100h
    mov es, ax
    pop ax
    mov bx, 2
    mul bx ; 每一个FAT项是两字节，给ax乘2就是偏移
LABEL_GET_FAT_ENTRY:
    ; 将ax变为扇区号
    xor dx, dx
    mov bx, [BPB_BytsPerSec]
    div bx ; dx = ax % 512, ax /= 512
    push dx ; 保存dx的值
    mov bx, 0 ; es:bx已指定
    add ax, SectorNoOfFAT1 ; 对应扇区号
    mov cl, 1 ; 一次读一个扇区即可
    call ReadSector ; 直接读入
    ; bx 到 bx + 512 处为读进扇区
    pop dx
    add bx, dx ; 加上偏移
    mov ax, [es:bx] ; 读取，那么这里就是了
LABEL_GET_FAT_ENTRY_OK: ; 胜利执行
    pop bx
    pop es ; 恢复堆栈
    ret
 
times 510 - ($ - $$) db 0
db 0x55, 0xaa ; 确保最后两个字节是0x55AA

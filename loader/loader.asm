    org 0100h ; 告诉编译器程序将装载至0x100处
 
BaseOfStack                 equ 0100h ; 栈的基址
BaseOfKernelFile            equ 08000h ; Kernel的基址
OffsetOfKernelFile          equ 0h  ; Kernel的偏移

    jmp LABEL_START
 
%include "fat16hdr.inc"
%include "load.inc"
%include "pm.inc"
%include "vbe.inc"

vbe_mode_num: dw 0 

vmode equ 0x0ff0
scrnx equ 0x0ff2
scrny equ 0x0ff4
vram equ 0x0ff6

xsize dw 0
ysize dw 0

selece_option db 2

welcome:
	db "Neumann Operating System",13,10
	db "Version 0.8 [Beta 6]",13,10
	db "Copyright(c) 2023-2025 W24 Studio & 71GN Deep Space",13,10
	db 0
menu:
	db 13,10
	db "Please select a display mode:",13,10
	db "1.VESA VBE  640 x 480  x 32bits [For 4 : 3 monitor]",13,10
	db "2.VESA VBE  800 x 600  x 32bits [For 4 : 3 monitor]",13,10
	db "3.VESA VBE 1024 x 768  x 32bits [For 4 : 3 monitor]",13,10
	db "4.VESA VBE 1280 x 1024 x 32bits [For 5 : 4 monitor]",13,10
    db "5.VESA VBE 1280 x 720  x 32bits [For 16: 9 monitor]",13,10
    db "6.VESA VBE 1600 x 900  x 32bits [For 16: 9 monitor]",13,10
    db "7.VESA VBE 1920 x 1080 x 32bits [For 16: 9 monitor]",13,10
    db "8.VESA VBE 2560 x 1440 x 32bits [For 16: 9 monitor]",13,10
    db "9.Reboot",13,10
    db "A.Shut Down",13,10
	db "Select:",0

vbe_not_support db "This resolution is not suitable for your computer. Please check your graphics card VBE version (must be 2.0 or above), or if your graphics card supports this resolution.",0
vga_driver db "Alternatively, can you use a VGA driver(Y/N,Selecting N will restart)?",0

LABEL_START:


    mov ax, cs
    mov ds, ax
    mov es, ax ; 将ds es设置为cs的值（因为此时字符串和变量等存在代码段内）
    mov ss, ax ; 将堆栈段也初始化至cs
    mov sp, BaseOfStack ; 设置栈顶

    mov dh, 0
    call DispStr ; Loading
 
    mov word [wSectorNo], SectorNoOfRootDirectory ; 开始查找，将当前读到的扇区数记为根目录区的开始扇区（19）
    xor ah, ah ; 复位
    xor dl, dl
    int 13h ; 执行软驱复位
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
    cmp word [wRootDirSizeForLoop], 0 ; 将剩余的根目录区扇区数与0比较
    jz LABEL_NO_KERNELBIN ; 相等，不存在Kernel，进行善后
    dec word [wRootDirSizeForLoop] ; 减去一个扇区
    mov ax, BaseOfKernelFile
    mov es, ax
    mov bx, OffsetOfKernelFile ; 将es:bx设置为BaseOfKernel:OffsetOfKernel，暂且使用Kernel所占的内存空间存放根目录区
    mov ax, [wSectorNo] ; 起始扇区：当前读到的扇区数（废话）
    mov cl, 1 ; 读取一个扇区
    call ReadSector ; 读入
 
    mov si, KernelFileName ; 为比对做准备，此处是将ds:si设为Kernel文件名
    mov di, OffsetOfKernelFile ; 为比对做准备，此处是将es:di设为Kernel偏移量（即根目录区中的首个文件块）
    cld ; FLAGS.DF=0，即执行lodsb/lodsw/lodsd后，si自动增加
    mov dx, 10h ; 共16个文件块（代表一个扇区，因为一个文件块32字节，16个文件块正好一个扇区）
LABEL_SEARCH_FOR_KERNELBIN:
    cmp dx, 0 ; 将dx与0比较
    jz LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR ; 继续前进一个扇区
    dec dx ; 否则将dx减1
    mov cx, 11 ; 文件名共11字节
LABEL_CMP_FILENAME: ; 比对文件名
    cmp cx, 0 ; 将cx与0比较
    jz LABEL_FILENAME_FOUND ; 若相等，说明文件名完全一致，表示找到，进行找到后的处理
    dec cx ; cx减1，表示读取1个字符
    lodsb ; 将ds:si的内容置入al，si加1
    cmp al, byte [es:di] ; 此字符与KERNEL  BIN中的当前字符相等吗？
    jz LABEL_GO_ON ; 下一个文件名字符
    jmp LABEL_DIFFERENT ; 下一个文件块
LABEL_GO_ON:
    inc di ; di加1，即下一个字符
    jmp LABEL_CMP_FILENAME ; 继续比较

LABEL_DIFFERENT:
    and di, 0FFE0h ; 指向该文件块开头
    add di, 20h ; 跳过32字节，即指向下一个文件块开头
    mov si, KernelFileName ; 重置ds:si
    jmp LABEL_SEARCH_FOR_KERNELBIN ; 由于要重新设置一些东西，所以回到查找Kernel循环的开头

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
    add word [wSectorNo], 1 ; 下一个扇区
    jmp LABEL_SEARCH_IN_ROOT_DIR_BEGIN ; 重新执行主循环

LABEL_NO_KERNELBIN: ; 若找不到kernel.bin则到这里
    mov dh, 2
    call DispStr ; 显示No KERNEL
    jmp $

LABEL_FILENAME_FOUND:
    mov ax, RootDirSectors ; 将ax置为根目录首扇区（19）
    and di, 0FFF0h ; 将di设置到此文件块开头
 
    push eax
    mov eax, [es:di + 01Ch]
    mov dword [dwKernelSize], eax
    pop eax
 
    add di, 01Ah ; 此时的di指向Kernel的FAT号
    mov cx, word [es:di] ; 获得该扇区的FAT号
    push cx ; 将FAT号暂存
    add cx, ax ; +根目录首扇区
    add cx, DeltaSectorNo ; 获得真正的地址
    mov ax, BaseOfKernelFile
    mov es, ax
    mov bx, OffsetOfKernelFile ; es:bx：读取扇区的缓冲区地址
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
    call ReadSector ; 读取Kernel第一个扇区
    pop ax ; 加载FAT号
    call GetFATEntry ; 加载FAT项
    cmp ax, 0FFFFh
    jz LABEL_FILE_LOADED ; 若此项=0FFF，代表文件结束，直接跳入Kernel
    push ax ; 重新存储FAT号，但此时的FAT号已经是下一个FAT了
    mov dx, RootDirSectors
    add ax, dx ; +根目录首扇区
    add ax, DeltaSectorNo ; 获取真实地址
    add bx, [BPB_BytsPerSec] ; 将bx指向下一个扇区开头
    jmp LABEL_GOON_LOADING_FILE ; 加载下一个扇区

VIDEO_MEMORY_ADDR_SEG equ 0xb800

menu_update:
    mov ax,VIDEO_MEMORY_ADDR_SEG
    mov gs,ax
    mov cx,10
    mov bx,0

.l1:
    push cx
    mov di,801
    mov ax,160
    mul bx
    add di,ax
    mov ax,0
    mov al,[selece_option]
    cmp bx,ax
    je .a
    mov al,0x07
    jmp .b
.a:
    mov al,0x70
.b:
    mov cx,51
.l2:
    mov byte[gs:di],al
    add di,2
    loop .l2
    inc bx
    pop cx
    loop .l1
    ret


LABEL_FILE_LOADED:
 
    mov dh, 1 ; "Ready."
 
   
    call DispStr ; Booting
     mov dh, 0
    call DispStr

    call cls

LABEL_MENU:

    mov si,welcome
    call putstr
    mov si,menu
    call putstr
.l1:

    call menu_update

    mov ah,0
    int 16h

    cmp ah,48h
    jz .upkey
    cmp ah,50h
    jz .downkey
    cmp al,0dh
    jz .done
    cmp al,"1"
    jz .option1
    cmp al,"2"
    jz .option2
    cmp al,"3"
    jz .option3
    cmp al,"4"
    jz .option4
    cmp al,"5"
    jz .option5
    cmp al,"6"
    jz .option6
    cmp al,"7"
    jz .option7
    cmp al,"8"
    jz .option8
    cmp al,"9"
    jz .option9
    cmp al,"a"
    jz .option10
    jmp .l1

.upkey:
    mov ax,0
    mov al,[selece_option]
    cmp al,0
    jz .l1
    dec al
    mov [selece_option],al
    jmp .l1

.downkey:
    mov ax,0
    mov al,[selece_option]
    cmp al,9
    jz .l1
    inc al
    mov [selece_option],al
    jmp .l1

.option1:
    mov byte[selece_option],0
    jmp .l1
.option2:
    mov byte[selece_option],1
    jmp .l1
.option3:
    mov byte[selece_option],2
    jmp .l1
.option4:
    mov byte[selece_option],3
    jmp .l1
.option5:
    mov byte[selece_option],4
    jmp .l1
.option6:
    mov byte[selece_option],5
    jmp .l1
.option7:
    mov byte[selece_option],6
    jmp .l1
.option8:
    mov byte[selece_option],7
    jmp .l1
.option9:
    mov byte[selece_option],8
    jmp .l1
.option10:
    mov byte[selece_option],9
    jmp .l1



.done:
    mov al,[selece_option]
    cmp al,0
    jz .vbe1
    cmp al,1
    jz .vbe2
    cmp al,2
    jz .vbe3
    cmp al,3
    jz .vbe4
    cmp al,4
    jz .vbe5
    cmp al,5
    jz .vbe6
    cmp al,6
    jz .vbe7
    cmp al,7
    jz .vbe8
    cmp al,8
    jz reboot
    cmp al,9
    jz shutdown

.vbe1:
    mov word[xsize],640
    mov word[ysize],480
    jmp LABLE_SETVBE
.vbe2:
    mov word[xsize],800
    mov word[ysize],600
    jmp LABLE_SETVBE
.vbe3:
    mov word[xsize],1024
    mov word[ysize],768
    jmp LABLE_SETVBE
.vbe4:
    mov word[xsize],1280
    mov word[ysize],1024
    jmp LABLE_SETVBE
.vbe5:
    mov word[xsize],1280
    mov word[ysize],720
    jmp LABLE_SETVBE
.vbe6:
    mov word[xsize],1600
    mov word[ysize],900
    jmp LABLE_SETVBE
.vbe7:
    mov word[xsize],1920
    mov word[ysize],1080
    jmp LABLE_SETVBE
.vbe8:
    mov word[xsize],2560
    mov word[ysize],1440
    jmp LABLE_SETVBE
shutdown:
	mov	ax,5301h
	xor	bx,bx
	int	15h
	mov	ax,530eh
	mov	cx,0102h
	int	15h
	mov	ax,5307h
	mov	bl,01h
	mov	cx,0003h
	int	15h

reboot:
	jmp 0xffff:0000


LABLE_SETVBE:
    mov di,0x2000 ;保存VbeInfoBlock的地方，占用512B。
    mov ax,0x4f00
    int 0x10
    cmp ax,0x004f
    jne scrn320
    ;检查VBE版本
    mov ax,[es:di+4]
    cmp ax,0x0200
    jb scrn320
    mov fs,[0x2000+16]
    mov si,[0x2000+14]
    mov dx,0 ;循环计数
check_mode_num:
    mov di,0x2200 ;保存ModeInfoBlock的地方，占用256B。
    mov ax,0x4f01 ;取得画面模式信息
    mov cx,[fs:si] ;模式号
    int 0x10
    cmp ax,0x004f
    jne next_mode_num
    cmp byte [es:di+0x19],32 ;颜色位数
    jne next_mode_num
    cmp byte [es:di+0x1b],6 ;颜色的指定方法（4是调色板模式，6是Direct Color）
    jne next_mode_num
    mov ax,[es:di+0x00] ;模式属性
    and ax,0x0080
    jz next_mode_num      ;模式属性的bit7是0，所以放弃。bit7是线性帧缓存区模式，1为有效，0为无效。
    mov ax,word[xsize];
    cmp word [es:di+18],ax ;水平分辨率
    jnz next_mode_num
    mov ax,word[ysize];
    cmp word [es:di+20],ax ;垂直分辨率
    jnz next_mode_num
save_vbe_data: 
    mov ax,[fs:si] ;保存模式号
    mov [vbe_mode_num],ax
    jmp set_vbe

next_mode_num:
    add si,2 ;指向下一个模式号
    inc dx
    cmp dx,111 ;存放模式号的空间共222字节，每个模式号占2字节，所以模式号数量不会超过111个。
    jb check_mode_num
set_vbe:
    mov bx,[vbe_mode_num]
    add bx,0x4000
    mov ax,0x4f02
    int 0x10
    cmp ax,0x004f ;如果返回值为0x004f说明设置显示模式成功
    jnz scrn320
save_vbe_info:
    mov ax,0x00
    mov gs,ax
    mov word[gs:vmode],COLOR_BITS ;
    mov ax,[es:di+0x12]
    mov word[gs:scrnx],ax  ; x分辨率
    mov ax,[es:di+0x14]
    mov word[gs:scrny],ax  ; y分辨率
    mov eax,[es:di+0x28]
    mov dword[gs:vram],eax
    jmp LABLE_GET_ARDS
scrn320:
    ; mov al,0x13 ;VGA显卡，320x200x8位彩色
    ; mov ah,0
    ; int 0x10
    call cls
    mov si,vbe_not_support
    call putstr
    mov ah,0eh
    mov al,13
    int 10h
    mov ah,0eh
    mov al,10
    int 10h
    mov si,vga_driver
    call putstr
    mov ah,00h
    int 16h
    cmp al,'n'
    jz reboot
    cmp al,'N'
    jz reboot
    cmp al,'Y'
    jz vga
    cmp al,'y'
    jz vga
    jmp $

vga:
    mov ah,00h
    mov al,13h
    int 10h

    mov ax,0x00
    mov gs,ax
    mov byte [vmode],8 ;
    mov ax,320
    mov word[gs:scrnx],ax  ; x分辨率
    mov ax,200
    mov word[gs:scrny],ax  ; y分辨率
    mov eax,0x000a0000
    mov dword[gs:vram],eax
    jmp LABLE_GET_ARDS
stop16:
    hlt 
    jmp stop16

LABLE_GET_ARDS:
    ; 将 ebx 置为 0
    xor ebx, ebx

    ; es:di 结构体的缓存位置
    mov ax, 0
    mov es, ax
    mov edi, 0x5000

    mov edx, 0x534d4150; 固定签名

.next:
    ; 子功能号
    mov eax, 0xe820
    ; ards 结构的大小 (字节)
    mov ecx, 20
    ; 调用 0x15 系统调用
    int 0x15

    ; 如果 CF 置位，表示出错
    jc .error

    ; 将缓存指针指向下一个结构体
    add di, cx

    ; 将结构体数量加一
    inc dword [ards_count]

    cmp ebx, 0
    jnz .next
    jmp LABLE_ENTER_PMODE
.error:
    mov si,load_error
    call putstr
.loop:
    hlt
    jmp .loop
    
load_error db "Load Error!",0

LABLE_ENTER_PMODE:
    lgdt [GdtPtr] ; 下面开始进入保护模式
 
    cli ; 关中断
 
    in al, 92h ; 使用A20快速门开启A20
    or al, 00000010b
    out 92h, al
 
    mov eax, cr0
    or eax, 1 ; 置位PE位
    mov cr0, eax
 
    jmp dword SelectorFlatC:(BaseOfLoaderPhyAddr + LABEL_PM_START) ; 真正进入保护模式
 
dwKernelSize        dd 0              ; Kernel大小
wRootDirSizeForLoop dw RootDirSectors ; 查找Kernel的循环中将会用到
wSectorNo           dw 0              ; 用于保存当前扇区数
bOdd                db 0              ; 这个其实是下一节的东西，不过先放在这也不是不行
 
KernelFileName      db "PRELOAD BIN", 0 ; Kernel的文件名

MessageLength       equ 9 ; 下面是三条小消息，此变量用于保存其长度，事实上在内存中它们的排序类似于二维数组
BootMessage:        db "Loading  " ; 此处定义之后就可以删除原先定义的BootMessage字符串了
Message1            db "Ready.   " ; 显示已准备好
Message2            db "No KERNEL" ; 显示没有Kernel

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
    add dh, 3
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

LABEL_GDT:          Descriptor 0,            0, 0                            ; 占位用描述符
LABEL_DESC_FLAT_C:  Descriptor 0,      0fffffh, DA_C | DA_32 | DA_LIMIT_4K   ; 32位代码段，平坦内存
LABEL_DESC_FLAT_RW: Descriptor 0,      0fffffh, DA_DRW | DA_32 | DA_LIMIT_4K ; 32位数据段，平坦内存
LABEL_DESC_VIDEO:   Descriptor 0B8000h, 0ffffh, DA_DRW | DA_DPL3             ; 文本模式显存，后面用不到了
 
GdtLen equ $ - LABEL_GDT                                                    ; GDT的长度
GdtPtr dw GdtLen - 1                                                        ; gdtr寄存器，先放置长度
       dd BaseOfLoaderPhyAddr + LABEL_GDT                                   ; 保护模式使用线性地址，因此需要加上程序装载位置的物理地址（BaseOfLoaderPhyAddr）
 
SelectorFlatC       equ LABEL_DESC_FLAT_C  - LABEL_GDT                      ; 代码段选择子
SelectorFlatRW      equ LABEL_DESC_FLAT_RW - LABEL_GDT                      ; 数据段选择子
SelectorVideo       equ LABEL_DESC_VIDEO   - LABEL_GDT + SA_RPL3            ; 文本模式显存选择子


putstr:
; 打印字符串
; 寄存器：in:SI
	mov	al,[si]
	cmp	al,0	; 如果[SI]=0
	je	.end	; 就结束
	mov	ah,0eh
	int	10h
	inc	si
	jmp	putstr
.end:
	ret

cls:
	; cls命令执行
	mov	ah,00h
	mov	al,03h
	int	10h
	mov ah,01h
	mov cx,0607H
	int 10h
	ret

ards_count:
    dd 0

[section .s32]
align 32
[bits 32]
LABEL_PM_START:
    mov ax, SelectorVideo ; 按照保护模式的规矩来
    mov gs, ax            ; 把选择子装入gs
 
    mov ax, SelectorFlatRW ; 数据段
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov esp, TopOfStack
 
; cs的设定已在之前的远跳转中完成
 
    call InitKernel
    jmp SelectorFlatC:KernelEntryPointPhyAddr

    jmp $
MemCpy: ; ds:参数2 ==> es:参数1，大小：参数3
    push ebp
    mov ebp, esp ; 保存ebp和esp的值
 
    push esi
    push edi
    push ecx ; 暂存这三个，要用
 
    mov edi, [ebp + 8] ; [esp + 4] ==> 第一个参数，目标内存区
    mov esi, [ebp + 12] ; [esp + 8] ==> 第二个参数，源内存区
    mov ecx, [ebp + 16] ; [esp + 12] ==> 第三个参数，拷贝的字节大小
.1:
    cmp ecx, 0 ; if (ecx == 0)
    jz .2 ; goto .2;
 
    mov al, [ds:esi] ; 从源内存区中获取一个值
    inc esi ; 源内存区地址+1
    mov byte [es:edi], al ; 将该值写入目标内存
    inc edi ; 目标内存区地址+1
 
    dec ecx ; 拷贝字节数大小-1
    jmp .1 ; 重复执行
.2:
    mov eax, [ebp + 8] ; 目标内存区作为返回值
 
    pop ecx ; 以下代码恢复堆栈
    pop edi
    pop esi
    mov esp, ebp
    pop ebp
 
    ret


 InitKernel:
    xor esi, esi ; esi = 0;
    mov cx, word [BaseOfKernelFilePhyAddr + 2Ch] ; 这个内存地址存放的是ELF头中的e_phnum，即Program Header的个数
    movzx ecx, cx ; ecx高16位置0，低16位置入cx
    mov esi, [BaseOfKernelFilePhyAddr + 1Ch] ; 这个内存地址中存放的是ELF头中的e_phoff，即Program Header表的偏移
    add esi, BaseOfKernelFilePhyAddr ; Program Header表的具体位置
.Begin:
    mov eax, [esi] ; 首先看一下段类型
    cmp eax, 0 ; 段类型：PT_NULL或此处不存在Program Header
    jz .NoAction ; 本轮循环不执行任何操作
    ; 否则的话：
    push dword [esi + 010h] ; p_filesz
    mov eax, [esi + 04h] ; p_offset
    add eax, BaseOfKernelFilePhyAddr ; BaseOfKernelFilePhyAddr + p_offset
    push eax
    push dword [esi + 08h] ; p_vaddr
    call MemCpy ; 执行一次拷贝
    add esp, 12 ; 清理堆栈
.NoAction: ; 本轮循环的清理工作
    add esi, 020h ; 下一个Program Header
    dec ecx
    jnz .Begin ; jz过来的话就直接ret了
 
    ret
[section .data1]
StackSpace: times 1024 db 0 ; 栈暂且先给1KB
TopOfStack  equ $ - StackSpace ; 栈顶
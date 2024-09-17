;nasmfunc.asm
;调用汇编函数
;Copyright W24 Studio 
[BITS 32]
GLOBAL asm_hlt,asm_cli,asm_sti,asm_stihlt
GLOBAL InitKernel,_IN


asm_hlt:
	hlt
	ret

asm_cli:
	cli
	ret

asm_sti:
	sti
	ret

asm_stihlt:
	sti
	hlt
	ret

BaseOfLoader            equ 09000h ; Loader的基址
OffsetOfLoader          equ 0100h  ; Loader的偏移

BaseOfLoaderPhyAddr     equ BaseOfLoader * 10h ; Loader被装载到的物理地址

BaseOfKernelFile            equ 08000h ; Kernel的基址
OffsetOfKernelFile          equ 0h  ; Kernel的偏移

BaseOfKernelFilePhyAddr     equ BaseOfKernelFile * 10h ; Kernel被装载到的物理地址
KernelEntryPointPhyAddr     equ 0x30400; Kernel入口点，一定要与编译命令一致！！！

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

_IN:
	mov  ebx, [esp + 4]
	mov  edx, [esp + 8]
	push ebx
	push edx
	jmp  far [esp]
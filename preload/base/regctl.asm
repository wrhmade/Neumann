;regctl.asm
;寄存器操作（汇编）
;Copyright W24 Studio 

[BITS 32]
global load_eflags,load_cr0,load_tr
global store_eflags,store_cr0

load_eflags:
    pushfd ; eflags寄存器只能用pushfd/popfd操作，将eflags入栈/将栈中内容弹入eflags
    pop eax ; eax = eflags;
    ret ; return eax;
 
store_eflags:
    mov eax, [esp + 4] ; 获取参数
    push eax
    popfd ; eflags = eax;
    ret
 
load_cr0:
    mov eax, cr0 ; cr0只能和eax之间mov
    ret ; return cr0;
 
store_cr0:
    mov eax, [esp + 4] ; 获取参数
    mov cr0, eax ; 赋值cr0
    ret

load_tr:
    ltr [esp + 4]
    ret
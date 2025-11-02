;syscall.asm
;系统调用(汇编)
;Copyright W24 Studio 
[BITS 32]
extern syscall_nmanager
global syscall_nhandler

syscall_nhandler:
    sti
    push ds
    push es
    pushad
    pushad

    mov ax, 0x10 ; 新增
    mov ds, ax   ; 新增
    mov es, ax   ; 新增

    call syscall_nmanager

    add esp, 32
    popad
    pop es
    pop ds
    iretd
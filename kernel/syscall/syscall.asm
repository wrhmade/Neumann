;syscall.asm
;系统调用(汇编)
;Copyright W24 Studio 
[BITS 32]
extern syscall_manager
global syscall_handler

syscall_handler:
    sti ; CPU 在执行 int 指令时默认关闭中断，我们只是来用一下系统功能，所以把中断打开
    pushad ; 用于返回值的 pushad
    pushad ; 用于给 syscall_manager 传值的 pushad

    call syscall_manager

    add esp, 32 ; 把给syscall_manager 传值的 pushad 部分跳过
    popad ; 把希望系统调用后的寄存器情况 pop 出来
    iretd ; 由于是 int 指令，所以用 iretd 返回
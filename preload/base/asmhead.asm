[section .bss]
; 这里，为栈准备空间
StackSpace resb 2 * 1024 ; 2KB的栈，大概够用？
StackTop: ; 栈顶位置
 
[section .text]
 
extern PRELDRMAIN ; kernel_main是C部分的主函数
global entry ; 真正的入口点

entry:
    mov esp, StackTop ; 先把栈移动过来
 
    cli ; 以防万一，再关闭一次中断（前面进保护模式已经关闭过一次）
    call PRELDRMAIN ; 进入krnlc_main
    jmp $ ; 从kernel_main回来了（一般不会发生），悬停


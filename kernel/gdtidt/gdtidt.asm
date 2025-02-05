;gdtidt.asm
;GDT&IDT管理程序（汇编）
;Copyright W24 Studio 

global gdt_flush,idt_flush
gdt_flush:
    mov eax, [esp + 4] ; 根据C编译器约定，C语言传入的第一个参数位于内存esp + 4处，第二个位于esp + 8处，以此类推，第n个位于esp + n * 4处
    lgdt [eax] ; 加载gdt并重新设置
; 接下来重新设置各段
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax ; 所有数据段均使用2号数据段
    jmp 0x08:.flush ; 利用farjmp重置代码段为1号代码段并刷新流水线
.flush:
    ret ; 完成

idt_flush:
    mov eax, [esp + 4]
    lidt [eax]
    ret
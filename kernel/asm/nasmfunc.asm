;nasmfunc.asm
;调用汇编函数
;Copyright W24 Studio 
[BITS 32]
GLOBAL asm_hlt,asm_cli,asm_sti,asm_stihlt
GLOBAL start_app
GLOBAL asm_end_app

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

start_app: ; void start_app(int new_eip, int new_cs, int new_esp, int new_ss, int *esp0)
    pushad
    mov eax, [esp + 36] ; new_eip
    mov ecx, [esp + 40] ; new_cs
    mov edx, [esp + 44] ; new_esp
    mov ebx, [esp + 48] ; new_ss
    mov ebp, [esp + 52] ; esp0
    mov [ebp], esp ; *esp0 = esp
    mov [ebp + 4], ss ; *ss0 = ss
; 用新的ss重设各段，实际上并不太合理而应使用ds
    mov es, bx
    mov ds, bx
    mov fs, bx
    mov gs, bx
; 选择子或上3表示要进入r3的段
    or ecx, 3 ; new_cs.RPL=3
    or ebx, 3 ; new_ss.RPL=3
    push ebx ; new_ss
    push edx ; new_esp
    push ecx ; new_cs
    push eax ; new_eip
    retf ; 剩下的弹出的活交给 CPU 来完成

asm_end_app:
    mov esp,[eax]
    mov dword[eax+4],0
    popad
    ret
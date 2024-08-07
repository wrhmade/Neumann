;nasmfunc.asm
;调用汇编函数
;Copyright W24 Studio 
[BITS 32]
GLOBAL asm_hlt,asm_cli,asm_sti,asm_stihlt

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
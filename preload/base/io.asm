;io.asm
;I/O端口操作
;Copyright W24 Studio 

[BITS 32]
GLOBAL io_in8,io_in16,io_in32
GLOBAL io_out8,io_out16,io_out32

io_in8:
		mov		edx,[esp+4]
		mov		eax,0
		in		al,dx
		ret

io_in16:
		mov		edx,[esp+4]
		mov		eax,0
		in		ax,dx
		ret

io_in32:
		mov		edx,[esp+4]
		mov		eax,0
		in		eax,dx
		ret

io_out8:
		mov		edx,[esp+4]
		mov		al,[esp+8]
		out		dx,al
		ret

io_out16:
		mov		edx,[esp+4]
		mov		eax,[esp+8]
		out		dx,ax
		ret

io_out32:
		mov		edx,[esp+4]
		mov		eax,[esp+8]
		out		dx,eax
		ret
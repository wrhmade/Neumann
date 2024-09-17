;mm.asm
;内存管理程序(汇编)
;Copyright W24 Studio 

[BITS 32]
global memtest_sub

memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		push	edi						; 乮ebx, esi, edi 傕巊偄偨偄偺偱乯
		push	esi
		push	ebx
		mov		esi,0xAA55AA55			; pat0 = 0xaa55aa55;
		mov		edi,0x55AA55AA			; pat1 = 0x55aa55aa;
		mov		eax,[esp+12+4]			; i = start;
mts_loop:
		mov		ebx,eax
		add		ebx,0xffc				; p = i + 0xffc;
		mov		edx,[ebx]				; old = *p;
		mov		[ebx],esi				; *p = pat0;
		xor		dword [ebx],0xffffffff	; *p ^= 0xffffffff;
		cmp		edi,[ebx]				; if (*p != pat1) goto fin;
		jne		mts_fin
		xor		dword [ebx],0xffffffff	; *p ^= 0xffffffff;
		cmp		esi,[ebx]				; if (*p != pat0) goto fin;
		jne		mts_fin
		mov		[ebx],edx				; *p = old;
		add		eax,0x1000				; i += 0x1000;
		cmp		eax,[esp+12+8]			; if (i <= end) goto mts_loop;
		jbe		mts_loop
		pop		ebx
		pop		esi
		pop		edi
		ret
mts_fin:
		mov		[ebx],edx				; *p = old;
		pop		ebx
		pop		esi
		pop		edi
		ret


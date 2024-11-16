[BITS 32]
a:
    mov eax,1
    mov ebx,mess
    int 80h
    mov eax,6
    mov ebx,50
    int 80h
    mov eax,0
    mov ebx,'A'
    int 80h
    jmp a

mess db "Loading...",0
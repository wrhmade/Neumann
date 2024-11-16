[BITS 32]
global api_movcur

api_movcur:
    push ebx
    mov eax,3
    mov ebx,0
    mov bh,[esp+8]
    mov bl,[esp+12]
    int 80h
    pop ebx
    ret
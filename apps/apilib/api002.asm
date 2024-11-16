[BITS 32]
global api_putstr

api_putstr:
    push ebx
    mov eax,1
    mov ebx,[esp+8]
    int 80h
    pop ebx
    ret
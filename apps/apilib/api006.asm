[BITS 32]
global api_getconsxy

api_getconsxy:
    push ebx
    mov eax,5
    int 80h
    pop ebx
    ret
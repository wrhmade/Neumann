[BITS 32]
global api_getkey

api_getkey:
    push ebx
    mov eax,4
    int 80h
    pop ebx
    ret
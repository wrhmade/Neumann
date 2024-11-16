[BITS 32]
global api_cls

api_cls:
    push ebx
    mov eax,2
    int 80h
    pop ebx
    ret
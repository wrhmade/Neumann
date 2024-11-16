[BITS 32]
global api_wait

api_wait:
    push ebx
    mov eax,6
    mov ebx,[esp+8]
    int 80h
    pop ebx
    ret
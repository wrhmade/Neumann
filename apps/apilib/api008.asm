[BITS 32]
global api_beep

api_beep:
    push ebx
    mov eax,7
    mov ebx,[esp+8]
    int 80h
    pop ebx
    ret
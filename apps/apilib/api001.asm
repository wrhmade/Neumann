[BITS 32]
global api_putchar

api_putchar:
    push ebx
    mov eax,0
    mov ebx,[esp+8]
    int 80h
    pop ebx
    ret
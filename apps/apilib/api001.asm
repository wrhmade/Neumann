[BITS 32]
global api_putchar

api_putchar:
    mov eax,0
    mov ebx,[esp+8]
    int 80h
    ret
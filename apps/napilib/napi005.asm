global napi_putchar
napi_putchar:
    mov eax,5
    mov ebx, [esp + 4]
    int 60h
    ret
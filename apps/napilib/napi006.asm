global napi_putchar_color
napi_putchar:
    mov eax,6
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int 60h
    ret
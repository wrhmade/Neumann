global napi_putstr_color
napi_putstr_color:
    mov eax,2
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int 60h
    ret
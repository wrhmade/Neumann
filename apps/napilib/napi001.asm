global napi_putstr
napi_putstr:
    mov eax,1
    mov ebx, [esp + 4]
    int 60h
    ret
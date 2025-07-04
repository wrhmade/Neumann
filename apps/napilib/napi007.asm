global napi_makewin
napi_makewin:
    mov eax,7
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    mov edx, [esp + 12]
    int 60h
    ret
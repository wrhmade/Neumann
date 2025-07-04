global napi_makewin_nobtn
napi_makewin_nobtn:
    mov eax,8
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    mov edx, [esp + 12]
    int 60h
    ret
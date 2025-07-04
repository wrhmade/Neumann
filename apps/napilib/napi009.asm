global napi_closewin
napi_closewin:
    mov eax,9
    mov ebx, [esp + 4]
    int 60h
    ret
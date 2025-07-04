global napi_exit
napi_exit:
    mov eax,0
    mov ebx, [esp + 4]
    int 60h
    ret
global napi_input
napi_input:
    mov eax,4
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    int 60h
    ret
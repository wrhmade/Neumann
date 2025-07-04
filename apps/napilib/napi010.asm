global napi_putstrwin
napi_putstrwin:
    mov eax,10
    mov edi, [esp + 4]
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    mov esi, [esp + 20]
    int 60h
    ret
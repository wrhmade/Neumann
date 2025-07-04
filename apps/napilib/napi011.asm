global napi_getcmdline
napi_getcmdline:
    mov eax,11
    mov ebx,[esp+4]
    int 60h
    ret
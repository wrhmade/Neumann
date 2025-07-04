global napi_getkey
napi_getkey:
    mov eax,3
    int 60h
    ret
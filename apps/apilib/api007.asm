[global unlink]
unlink:
    push ebx
    mov eax, 6
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret
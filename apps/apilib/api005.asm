[global close]
close:
    push ebx
    mov eax, 4
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret
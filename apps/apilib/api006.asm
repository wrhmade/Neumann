[global lseek]
lseek:
    push ebx
    mov eax, 5
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    int 80h
    pop ebx
    ret
[global create_process]
create_process:
    push ebx
    mov eax, 7
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    int 80h
    pop ebx
    ret
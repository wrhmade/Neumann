[global waitpid]
waitpid:
    push ebx
    mov eax, 8
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret
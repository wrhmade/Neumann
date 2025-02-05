[global exit]
exit:
    push ebx
    mov eax, 9
    mov ebx, [esp + 8]
    int 80h
    pop ebx
    ret
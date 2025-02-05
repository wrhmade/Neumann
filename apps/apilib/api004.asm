[global open]
open:
    push ebx
    mov eax, 3
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    int 80h
    pop ebx
    ret
    bits 32

    mov eax,3
    int 60h

    mov eax,1
    mov ebx,string
    int 60h

    mov eax,0
    mov ebx,0
    int 60h


    jmp $

string: db "Hello, World!", 0x0A, 0x00
strlen equ $ - string
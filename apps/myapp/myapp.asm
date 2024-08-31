[BITS 32]
section .code

global main

main:
    mov eax,0
    mov ebx,'A'
    int 80h

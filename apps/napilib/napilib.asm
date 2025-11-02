
%macro SYSCALL0 2
[global %1]
%1:
    mov eax, %2
    int 60h
    ret
%endmacro

%macro SYSCALL1 2
[global %1]
%1:
    push ebx
    mov eax, %2
    mov ebx, [esp + 8]
    int 60h
    pop ebx
    ret
%endmacro

%macro SYSCALL2 2
[global %1]
%1:
    push ebx
    mov eax, %2
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    int 60h
    pop ebx
    ret
%endmacro

%macro SYSCALL3 2
[global %1]
%1:
    push ebx
    mov eax, %2
    mov ebx, [esp + 8]
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    int 60h
    pop ebx
    ret
%endmacro

SYSCALL1 napi_exit,0
SYSCALL1 napi_putc,1
SYSCALL1 napi_puts,2
SYSCALL1 napi_heap_resize,3
SYSCALL0 napi_getc,4
SYSCALL2 napi_gets,5
SYSCALL0 napi_console_clearscreen,6
SYSCALL1 napi_getcurrenttime,7
SYSCALL2 napi_open_file,8
SYSCALL2 napi_file_status,9
SYSCALL2 npai_file_set_pos,10
SYSCALL3 napi_read_file,11
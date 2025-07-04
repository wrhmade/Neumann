section .text

%macro SYSCALL0 2
[global %1]
%1:
    mov eax, %2
    int 80h
    ret
%endmacro

%macro SYSCALL1 2
[global %1]
%1:
    push ebx
    mov eax, %2
    mov ebx, [esp + 8]
    int 80h
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
    int 80h
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
    int 80h
    pop ebx
    ret
%endmacro

SYSCALL0 getpid, 0
SYSCALL3 write, 1
SYSCALL3 read, 2
SYSCALL2 open, 3
SYSCALL1 close, 4
SYSCALL3 lseek, 5
SYSCALL1 unlink, 6
SYSCALL3 create_process, 7
SYSCALL1 waitpid, 8
SYSCALL1 exit, 9
SYSCALL1 sbrk, 10
SYSCALL1 opendir, 11
SYSCALL1 readdir, 12
SYSCALL1 rewinddir, 13
SYSCALL1 closedir, 14
SYSCALL1 mkdir, 15
SYSCALL1 rmdir, 16
SYSCALL2 fstat, 17
SYSCALL1 chdir, 18
SYSCALL2 getcwd, 19
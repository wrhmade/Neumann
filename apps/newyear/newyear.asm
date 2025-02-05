    bits 32

    mov eax, 1
    mov ebx, 1
    mov ecx, string
    mov edx, 114514
    int 80h




    
    mov eax, 9
    mov ebx, 114514
    int 80h

    jmp $

;¸£
string:
string1   db "                    /\", 0X0A
string2   db "                  /    \", 0X0A
string3   db "                /        \", 0X0A
string4   db "              /            \", 0X0A
string5   db "            /                \", 0X0A
string6   db "          /   a     aaaaaaa    \", 0X0A
string7   db "        /      a                 \", 0X0A
string8   db "      /    aaaaaaa   aaaaa         \", 0X0A
string9   db "    /           a    a   a           \", 0X0A
string10  db "  /            a     aaaaa             \", 0X0A
string11  db "/             aaa                        \", 0X0A
string12  db "\            a a a  aaaaaaa              /", 0X0A
string13  db "  \        a   a    a  a  a            /", 0X0A
string14  db "    \          a    aaaaaaa          /", 0X0A
string15  db "      \        a    a  a  a        /", 0X0A
string16  db "        \      a    aaaaaaa      /", 0X0A
string17  db "          \    a               /", 0X0A
string18  db "            \                /", 0X0A
string19  db "              \            /", 0X0A
string20  db "                \        /", 0X0A
string21  db "                  \    /", 0X0A
string22  db "                    \/", 0X0A
db 0x00

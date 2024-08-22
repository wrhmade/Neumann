;farjmp.asm
;farjmp实现
;Copyright W24 Studio 

global farjmp
farjmp:
    jmp far [esp + 4]
    ret
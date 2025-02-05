TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c task.c -o task.obj -I../include
	$(NASM) -f $(NASM_FORMAT) farjmp.asm -o farjmp.obj  -I../include -I../../include

clean:
	-$(DEL) task.obj
	-$(DEL) farjmp.obj
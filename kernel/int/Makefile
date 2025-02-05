TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c int.c -o int.obj -I../include
	$(NASM) -f $(NASM_FORMAT) int.asm -o intasm.obj 

clean:
	-$(DEL) int.obj
	-$(DEL) intasm.obj
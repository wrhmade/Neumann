TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c gdtidt.c -o gdtidt.obj -I../include
	$(NASM) -f $(NASM_FORMAT) gdtidt.asm -o gdtasm.obj

clean:
	-$(DEL) gdtidt.obj
	-$(DEL) gdtasm.obj
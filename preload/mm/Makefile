TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c mm.c -o mm.obj -I../include
	$(NASM) -f $(NASM_FORMAT) mm.asm -o mmasm.obj  -I../include -I../../include

clean:
	-$(DEL) mm.obj
	-$(DEL) mmasm.obj
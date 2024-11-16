TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c mm.c -o mm.obj -I../include
	$(GCC) -c page.c -o page.obj -I../include
	$(GCC) -c bitmap.c -o bitmap.obj -I../include
	$(NASM) -f $(NASM_FORMAT) mm.asm -o mmasm.obj  -I../include -I../../include

clean:
	-$(DEL) mm.obj
	-$(DEL) page.obj
	-$(DEL) bitmap.obj
	-$(DEL) mmasm.obj
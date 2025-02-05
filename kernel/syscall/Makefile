TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c syscall.c -o syscall.obj -I../include
	$(NASM) -f $(NASM_FORMAT) syscall.asm -o syscasm.obj -I../include -I../../include

clean:
	-$(DEL) syscall.obj
	-$(DEL) syscasm.obj
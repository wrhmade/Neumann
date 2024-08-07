TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(NASM) -f $(NASM_FORMAT) nasmfunc.asm -o nasmfunc.obj -I../include -I../../include

clean:
	-$(DEL) nasmfunc.obj

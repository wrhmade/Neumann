TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c preload.c -o preload.obj -I../include
	$(GCC) -c krnlcons.c -o krnlcons.obj -I../include
	$(NASM) -f $(NASM_FORMAT) nasmfunc.asm -o nasmfunc.obj  -I../include -I../../include
	$(NASM) -f $(NASM_FORMAT) regctl.asm -o regctl.obj  -I../include -I../../include
	$(NASM) -f $(NASM_FORMAT) io.asm -o io.obj  -I../include -I../../include
	$(NASM) -f $(NASM_FORMAT) asmhead.asm -o asmhead.obj  -I../include -I../../include

clean:
	-$(DEL) preload.obj
	-$(DEL) nasmfunc.obj
	-$(DEL) regctl.obj
	-$(DEL) io.obj
	-$(DEL) asmhead.obj
	-$(DEL) krnlcons.obj
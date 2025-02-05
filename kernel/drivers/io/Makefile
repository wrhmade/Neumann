TOOLPATH=../../../../tools/
include ../../../config.txt

default:
	$(NASM) -f $(NASM_FORMAT) io.asm -o io.obj -I../../include -I../../../include

clean:
	-$(DEL) io.obj
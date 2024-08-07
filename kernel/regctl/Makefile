TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(NASM) -f $(NASM_FORMAT) regctl.asm -o regctl.obj -I../include -I../../include

clean:
	-$(DEL) regctl.obj
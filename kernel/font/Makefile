TOOLPATH=../../../tools/
include ../../config.txt

FONTNAME = font.bin

default:
	$(NASM) -f $(NASM_FORMAT) font.asm -o font.obj

clean:
	-$(DEL) font.obj
TOOLPATH	=../../tools/
include ../config.txt

default:
	$(NASM) boot.asm -o boot.bin -I../include

clean:
	$(DEL) boot.bin
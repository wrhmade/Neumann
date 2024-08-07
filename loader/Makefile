TOOLPATH	=../../tools/
include ../config.txt

default:
	$(NASM) loader.asm -o loader.bin -I../include

clean:
	$(DEL) loader.bin
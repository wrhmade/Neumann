TOOLPATH=../../../../tools/
include ../../../config.txt

default:
	$(GCC) -I../../include -I../../../include -c audio.c -o audio.obj
	$(GCC) -I../../include -I../../../include -c sb16/sb16.c -o sb16/sb16.obj

clean:
	-$(DEL) audio.obj
	-$(DEL) sb16/sb16.obj
TOOLPATH=../../../../tools/
include ../../../config.txt

default:
	$(GCC) -I../../include -I../../../include -c keyboard.c -o keyboard.obj
	$(GCC) -I../../include -I../../../include -c keymap.c -o keymap.obj

clean:
	-$(DEL) keyboard.obj
	-$(DEL) keymap.obj
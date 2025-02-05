TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c jpeg.c -o jpeg.obj -I../include
	$(GCC) -c ini.c -o ini.obj -I../include
	$(GCC) -c maths.c -o maths.obj -I../include
	$(GCC) -c ime.c -o ime.obj -I../include
	$(GCC) -c msdemo.c -o msdemo.obj -I../include
	$(NASM) -f $(NASM_FORMAT) bmp.asm -o bmp.obj -I../include -I../../include

clean:
	-$(DEL) jpeg.obj
	-$(DEL) bmp.obj
	-$(DEL) ini.obj
	-$(DEL) maths.obj
	-$(DEL) msdemo.obj
	-$(DEL) ime.obj

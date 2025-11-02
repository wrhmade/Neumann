TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c ini.c -o ini.obj -I../include
	$(GCC) -c ime.c -o ime.obj -I../include
	$(GCC) -c msdemo.c -o msdemo.obj -I../include
	$(GCC) -c wav.c -o wav.obj -I../include
	$(GCC) -c screenshot.c -o screenshot.obj -I../include

clean:
	-$(DEL) ini.obj
	-$(DEL) msdemo.obj
	-$(DEL) ime.obj
	-$(DEL) wav.obj
	-$(DEL) screenshot.obj
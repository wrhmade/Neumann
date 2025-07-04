TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c myapp.c -o myapp.obj -I../include -fno-pic
	$(LD) myapp.obj ../start/start.obj ../lib/libn.a ../napilib/libnapi.a ../apilib/libapi.a -o myapp.bin -e _start -s -Ttext 0x0
clean:
	-$(DEL) myapp.bin
	-$(DEL) myapp.obj

TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c disk.c -o disk.obj -I../include
	$(GCC) -c vdisk.c -o vdisk.obj -I../include

clean:
	-$(DEL) disk.obj
	-$(DEL) vdisk.obj
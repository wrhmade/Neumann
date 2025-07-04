
TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c fat.c -o fat.obj -I../include
	$(GCC) -c vfs.c -o vfs.obj -I../include
	$(GCC) -c vfile.c -o vfile.obj -I../include
	$(GCC) -c devfs.c -o devfs.obj -I../include
	$(GCC) -c iso9660.c -o iso9660.obj -I../include
	$(MAKE) -C fatfs

clean:
	-$(DEL) fat16.obj
	-$(DEL) vfs.obj
	-$(DEL) vfile.obj
	-$(DEL) devfs.obj
	-$(DEL) iso9660.obj
	$(MAKE) -C fatfs clean
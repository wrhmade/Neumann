TOOLPATH=../../../../tools/
include ../../../config.txt

default:
	$(GCC) -c diskio.c -o diskio.obj -I../../include
	$(GCC) -c ff.c -o ff.obj -I../../include
	$(GCC) -c ffsystem.c -o ffsystem.obj -I../../include
	$(GCC) -c ffunicode.c -o ffunicode.obj -I../../include

clean:
	-$(DEL) fat16.obj
	-$(DEL) vfs.obj
	-$(DEL) vfile.obj
	-$(DEL) devfs.obj
	
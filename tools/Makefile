TOOLPATH	=../../tools/
include ../config.txt

default:
	$(MAKE) ftcopy
	$(MAKE) ftformat
	$(MAKE) ftimgcreate
	$(MAKE) ftls
	$(MAKE) ftrm
	$(MAKE) hexview
	$(MAKE) fvdisk

ftcopy:ftcopy.c
	$(GCC2) ftcopy.c -o ftcopy

ftformat:ftformat.c
	$(GCC2) ftformat.c -o ftformat

ftimgcreate:ftimgcreate.c
	$(GCC2) ftimgcreate.c -o ftimgcreate

ftls:ftls.c
	$(GCC2) ftls.c -o ftls

ftrm:ftrm.c
	$(GCC2) ftrm.c -o ftrm

hexview:hexview.c
	$(GCC2) hexview.c -o hexview

fvdisk:fvdisk.c
	$(GCC2) fvdisk.c -o fvdisk

clean:
	$(DEL) ftcopy
	$(DEL) ftformat
	$(DEL) ftimgcreate
	$(DEL) ftls
	$(DEL) ftrm
	$(DEL) hexview
	$(DEL) fvdisk
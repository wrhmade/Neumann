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
	$(GCC) ftcopy.c -o ftcopy

ftformat:ftformat.c
	$(GCC) ftformat.c -o ftformat

ftimgcreate:ftimgcreate.c
	$(GCC) ftimgcreate.c -o ftimgcreate

ftls:ftls.c
	$(GCC) ftls.c -o ftls

ftrm:ftrm.c
	$(GCC) ftls.c -o ftls

hexview:hexview.c
	$(GCC) hexview.c -o hexview

fvdisk:fvdisk.c
	$(GCC) fvdisk.c -o fvdisk

clean:
	$(DEL) ftcopy
	$(DEL) ftformat
	$(DEL) ftimgcreate
	$(DEL) ftls
	$(DEL) ftrm
	$(DEL) hexview
	$(DEL) fvdisk
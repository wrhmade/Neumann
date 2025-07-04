TOOLPATH	=../../tools/
include ../config.txt

default:
	$(MAKE) ftcopy
	$(MAKE) ftimage
	$(MAKE) ftls
	$(MAKE) hexview


ftcopy:ftcopy.c
	$(GCC2) ftcopy.c -o ftcopy

ftimage:ftimage.c
	$(GCC2) ftimage.c -o ftimage

ftls:ftls.c
	$(GCC2) ftls.c -o ftls


hexview:hexview.c
	$(GCC2) hexview.c -o hexview

clean:
	$(DEL) ftcopy
	$(DEL) ftimage
	$(DEL) ftls
	$(DEL) hexview
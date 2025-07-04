TOOLPATH=../tools/
include config.txt

default:
	$(MAKE) $(IMAGENAME)

$(IMAGENAME):Makefile boot/boot.bin loader/loader.bin kernel/kernel.bin
	$(FTIMAGE) $(IMAGENAME) -size 80 -bs boot/boot.bin
	$(FTCOPY) $(IMAGENAME) -srcpath loader/loader.bin -to -dstpath /loader.bin
	$(FTCOPY) $(IMAGENAME) -srcpath preload/preload.bin -to -dstpath /preload.bin
	$(FTCOPY) $(IMAGENAME) -srcpath kernel/kernel.bin -to -dstpath /kernel.bin
	$(FTCOPY) $(IMAGENAME) -srcpath kernel/font/HZK16.bin -to -dstpath /resource/font/HZK16.bin
	$(FTCOPY) $(IMAGENAME) -srcpath kernel/font/HZK16F.bin -to -dstpath /resource/font/HZK16F.bin
	$(FTCOPY) $(IMAGENAME) -srcpath res/neumann.ini -to -dstpath /config/neumann.ini
	$(FTCOPY) $(IMAGENAME) -srcpath res/pymb.dat -to -dstpath /resource/ime/pymb.dat
	$(FTCOPY) $(IMAGENAME) -srcpath res/console.ini -to -dstpath /config/console.ini
	# $(FTCOPY) $(IMAGENAME) -srcpath res/themes/default.tme -to -dstpath /resource/themes/default/default.tme
	# $(FTCOPY) $(IMAGENAME) -srcpath res/themes/default.jpg -to -dstpath /resource/themes/default/default.jpg
	# $(FTCOPY) $(IMAGENAME) -srcpath res/themes/summer.tme -to -dstpath /resource/themes/summer/summer.tme
	# $(FTCOPY) $(IMAGENAME) -srcpath res/themes/summer.jpg -to -dstpath /resource/themes/summer/summer.jpg
	$(FTCOPY) $(IMAGENAME) -srcpath apps/myapp/myapp.bin -to -dstpath /test/myapp.bin



full:
	$(MAKE) -C boot
	$(MAKE) -C loader
	$(MAKE) -C kernel
	$(MAKE) -C apps
	$(MAKE) -C tools
	$(MAKE) -C preload
	$(MAKE) $(IMAGENAME)

clean_full:
	$(DEL) $(IMAGENAME)
	$(DEL) $(IMAGENAME_VHD)
	$(DEL) $(IMAGENAME_VMDK)
	$(MAKE) -C boot clean
	$(MAKE) -C loader clean
	$(MAKE) -C kernel clean
	$(MAKE) -C tools clean
	$(MAKE) -C apps clean
	$(MAKE) -C preload clean

fastbuild:
	$(MAKE) clean_full
	$(MAKE) full
	$(MAKE) to_vhd
	$(MAKE) to_vmdk


run:
	$(QEMU) -hda $(IMAGENAME) -m $(MEMORY_SIZE) -serial stdio

to_vhd:
	$(QEMU_IMG) convert -f raw -O vpc $(IMAGENAME) $(IMAGENAME_VHD)

to_vmdk:
	$(QEMU_IMG) convert -f raw -O vmdk $(IMAGENAME) $(IMAGENAME_VMDK)

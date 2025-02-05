TOOLPATH=../tools/
include config.txt

default:
	$(MAKE) $(IMAGENAME)

$(IMAGENAME):Makefile boot/boot.bin loader/loader.bin kernel/kernel.bin
	$(FTIMGCREATE) $(IMAGENAME) -t hd -size 80
	$(FTFORMAT) $(IMAGENAME) -t hd -f fat16
	$(FVDISK) loader/loader.bin -o $(IMAGENAME) -f 0
	$(FVDISK) preload/preload.bin -o $(IMAGENAME) -f 0
	$(FVDISK) kernel/kernel.bin -o $(IMAGENAME) -f 0
	$(FVDISK) kernel/font/HZK16.bin -o $(IMAGENAME) -f 0
	$(FVDISK) kernel/font/HZK16F.bin -o $(IMAGENAME) -f 0
	$(FVDISK) res/test.txt -o $(IMAGENAME) -f 0
	$(FVDISK) res/neumann.ini -o $(IMAGENAME) -f 0
	$(FVDISK) res/test.prg -o $(IMAGENAME) -f 0
	$(FVDISK) res/print.txt -o $(IMAGENAME) -f 0
	$(FVDISK) res/print2.txt -o $(IMAGENAME) -f 0
	$(FVDISK) res/pymb.dat -o $(IMAGENAME) -f 0
	$(FVDISK) res/themes/default.tme -o $(IMAGENAME) -f 0
	$(FVDISK) res/themes/default.jpg -o $(IMAGENAME) -f 0
	$(FVDISK) res/themes/newyear.tme -o $(IMAGENAME) -f 0
	$(FVDISK) res/themes/newyear.jpg -o $(IMAGENAME) -f 0
	$(FVDISK) apps/myapp/myapp.bin -o $(IMAGENAME) -f 0
	$(FVDISK) apps/newyear/newyear.bin -o $(IMAGENAME) -f 0
	$(FVDISK) boot/boot.bin -o $(IMAGENAME) -s 0


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
	$(QEMU) -hda $(IMAGENAME) -m $(MEMORY_SIZE)

to_vhd:
	$(QEMU_IMG) convert -f raw -O vpc $(IMAGENAME) $(IMAGENAME_VHD)

to_vmdk:
	$(QEMU_IMG) convert -f raw -O vmdk $(IMAGENAME) $(IMAGENAME_VMDK)

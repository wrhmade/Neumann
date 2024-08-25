TOOLPATH=../tools/
include config.txt

default:
	$(MAKE) $(IMAGENAME)

$(IMAGENAME):Makefile boot/boot.bin loader/loader.bin kernel/kernel.bin
	$(FTIMGCREATE) $(IMAGENAME) -t hd -size 80
	$(FTFORMAT) $(IMAGENAME) -t hd -f fat16
	$(FTCOPY) loader/loader.bin -to -img $(IMAGENAME)
	$(FTCOPY) kernel/kernel.bin -to -img $(IMAGENAME)
	$(FVDISK) boot/boot.bin -o $(IMAGENAME) -s 0


full:
	$(MAKE) -C boot
	$(MAKE) -C loader
	$(MAKE) -C kernel
	$(MAKE) -C tools
	$(MAKE) $(IMAGENAME)

clean_full:
	$(DEL) $(IMAGENAME)
	$(DEL) $(IMAGENAME_VHD)
	$(DEL) $(IMAGENAME_VMDK)
	$(MAKE) -C boot clean
	$(MAKE) -C loader clean
	$(MAKE) -C kernel clean
	$(MAKE) -C tools clean

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
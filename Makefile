TOOLPATH=../tools/
include config.txt

default:
	$(MAKE) $(IMAGENAME)

$(IMAGENAME):Makefile boot/boot.bin loader/loader.bin kernel/kernel.bin
	$(QEMU_IMG) create -f raw $(IMAGENAME) 1474560
	$(MFORMAT) -f 1440 -B boot/boot.bin -i $(IMAGENAME)
	$(MCOPY) -i $(IMAGENAME) loader/loader.bin ::
	$(MCOPY) -i $(IMAGENAME) kernel/kernel.bin ::

full:
	$(MAKE) -C boot
	$(MAKE) -C loader
	$(MAKE) -C kernel
	$(MAKE) $(IMAGENAME)

clean_full:
	$(DEL) bootfd.img
	$(MAKE) -C boot clean
	$(MAKE) -C loader clean
	$(MAKE) -C kernel clean

fastbuild:
	$(MAKE) clean_full
	$(MAKE) full


run:
	$(QEMU) -fda $(IMAGENAME) -m $(MEMORY_SIZE)

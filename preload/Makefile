TOOLPATH=../../tools/
include ../config.txt

OBJS=base/asmhead.obj base/preload.obj base/nasmfunc.obj base/regctl.obj hd/hd.obj cmos/cmos.obj base/io.obj fs/fat16.obj mm/mm.obj mm/mmasm.obj lib/string/string.obj \
	lib/sprintf/sprintf.obj graphic/graphic.obj font/font.obj

default:
	$(MAKE) -C base
	$(MAKE) -C mm
	$(MAKE) -C lib
	$(MAKE) -C graphic
	$(MAKE) -C font
	$(MAKE) -C fs
	$(MAKE) -C cmos
	$(MAKE) -C hd
	$(LD) -s -Ttext 0x20000 -o preload.bin $(OBJS) -e entry

clean:
	$(MAKE) -C base clean
	$(MAKE) -C mm clean
	$(MAKE) -C lib clean
	$(MAKE) -C graphic clean
	$(MAKE) -C font clean
	$(MAKE) -C fs clean
	$(MAKE) -C cmos clean
	$(MAKE) -C hd clean
	$(DEL) preload.bin

	

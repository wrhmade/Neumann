TOOLPATH=../../tools/
include ../config.txt

OBJS=asmhead/asmhead.obj main/main.obj asm/nasmfunc.obj gui/graphic/graphic.obj gui/desktop/desktop.obj font/font.obj gdtidt/gdtidt.obj gdtidt/gdtasm.obj \
	int/int.obj int/intasm.obj lib/string/string.obj lib/sprintf/sprintf.obj drivers/io/io.obj drivers/mouse/mouse.obj drivers/keyboard/keyboard.obj fifo/fifo.obj \
	drivers/keyboard/keymap.obj mm/mm.obj mm/mmasm.obj regctl/regctl.obj gui/sheet/sheet.obj gui/window/window.obj task/task.obj task/farjmp.obj drivers/timer/timer.obj \
	drivers/buzzer/buzzer.obj user/console/console.obj fault/fault.obj drivers/hd/hd.obj drivers/cmos/cmos.obj syscall/syscall.obj syscall/syscasm.obj \
	plug-in/ini.obj drivers/com/com.obj drivers/lpt/lpt.obj ELF/elf.obj user/message/message.obj plug-in/msdemo.obj \
	plug-in/ime.obj krnlcons/krnlcons.obj drivers/fpu/fpu.obj drivers/cpu/cpu.obj drivers/pci/pci.obj exec/exec.obj gui/theme/theme.obj \
	lib/math/math.obj disk/disk.obj disk/vdisk.obj drivers/fdc/fdc.obj drivers/dma/dma.obj fs/vfs.obj fs/devfs.obj fs/vfile.obj lib/data_structures/slist-strptr.obj \
	lib/data_structures/rbtree-strptr.obj fs/fat.obj fs/fatfs/ff.obj fs/fatfs/ffsystem.obj fs/fatfs/ffunicode.obj fs/fatfs/diskio.obj\
	user/fullscreen/fullscreen.obj drivers/dbuffer/dbuffer.obj drivers/ide/ide.obj fs/iso9660.obj drivers/nullzero/nullzero.obj drivers/acpi/acpi.obj power/power.obj
	
default:
	$(MAKE) -C asmhead
	$(MAKE) -C main
	$(MAKE) -C asm
	$(MAKE) -C gui
	$(MAKE) -C font
	$(MAKE) -C gdtidt
	$(MAKE) -C int
	$(MAKE) -C lib
	$(MAKE) -C drivers
	$(MAKE) -C fifo
	$(MAKE) -C mm
	$(MAKE) -C regctl
	$(MAKE) -C task
	$(MAKE) -C fault
	$(MAKE) -C user
	$(MAKE) -C fs
	$(MAKE) -C syscall
	$(MAKE) -C ELF
	$(MAKE) -C plug-in
	$(MAKE) -C krnlcons
	$(MAKE) -C exec
	$(MAKE) -C disk
	$(MAKE) -C power
	$(LD) -Ttext 0x100000 -o kernel.bin $(OBJS) -e krnl_entry 

clean:
	$(MAKE) -C asmhead clean
	$(MAKE) -C main clean
	$(MAKE) -C asm clean
	$(MAKE) -C gui clean
	$(MAKE) -C font clean
	$(MAKE) -C gdtidt clean
	$(MAKE) -C int clean
	$(MAKE) -C lib clean
	$(MAKE) -C drivers clean
	$(MAKE) -C fifo clean
	$(MAKE) -C mm clean
	$(MAKE) -C regctl clean
	$(MAKE) -C task clean
	$(MAKE) -C fault clean
	$(MAKE) -C user clean
	$(MAKE) -C fs clean
	$(MAKE) -C syscall clean
	$(MAKE) -C plug-in clean
	$(MAKE) -C ELF clean
	$(MAKE) -C krnlcons clean
	$(MAKE) -C exec clean
	$(MAKE) -C disk clean
	$(MAKE) -C power clean
	$(DEL) kernel.bin

	

TOOLPATH=../../tools/
include ../config.txt

OBJS=asmhead/asmhead.obj main/main.obj asm/nasmfunc.obj gui/graphic/graphic.obj gui/desktop/desktop.obj font/font.obj gdtidt/gdtidt.obj gdtidt/gdtasm.obj \
	int/int.obj int/intasm.obj lib/string/string.obj lib/sprintf/sprintf.obj drivers/io/io.obj drivers/mouse/mouse.obj drivers/keyboard/keyboard.obj fifo/fifo.obj \
	drivers/keyboard/keymap.obj mm/mm.obj mm/mmasm.obj regctl/regctl.obj gui/sheet/sheet.obj gui/window/window.obj task/task.obj task/farjmp.obj drivers/timer/timer.obj \
	drivers/buzzer/buzzer.obj

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
	$(LD) -s -Ttext 0x100000 -o kernel.bin $(OBJS) -e krnl_entry

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
	$(DEL) kernel.bin

	

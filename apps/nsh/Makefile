TOOLPATH=../../../tools/
include ../../config.txt

OBJS=nsh.obj shell.obj

%.obj:%.c
	$(GCC) -c $*.c -o $*.obj -I../include -fno-pic

nsh.bin:$(OBJS)
	$(LD) $(OBJS) ../napilib/libnapi.a -o nsh.bin -e main -s -Ttext 0x0

clean:
	-$(DEL) $(OBJS)
	-$(DEL) nsh.bin
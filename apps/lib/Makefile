TOOLPATH=../../../tools/
include ../../config.txt

OBJS=stdio.obj string.obj stdlib.obj ctype.obj time.obj

%.obj:%.c
	$(GCC) -c $*.c -o $*.obj -I../include -fno-pic

libn.a:$(OBJS)
	$(AR) rv libn.a $(OBJS)  ../apilib/libapi.a ../napilib/libnapi.a

clean:
	-$(DEL) $(OBJS)
	-$(DEL) libn.a
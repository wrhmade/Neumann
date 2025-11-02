TOOLPATH=../../../tools/
include ../../config.txt

OBJS_NAPI =	napilib.obj

%.obj:%.asm
	$(NASM) -f $(NASM_FORMAT) $*.asm -o $*.obj

libapi.a:$(OBJS_NAPI)
	$(AR) rv libnapi.a $(OBJS_NAPI)

clean:
	-$(DEL) $(OBJS_NAPI)
	-$(DEL) libnapi.a
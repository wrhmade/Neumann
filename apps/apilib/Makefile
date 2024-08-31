TOOLPATH=../../../tools/
include ../../config.txt

OBJS_API =	api001.obj

%.obj:%.asm
	$(NASM) -f $(NASM_FORMAT) $*.asm -o $*.obj

libapi.a:$(OBJS_API)
	$(AR) rv libapi.a $(OBJS_API)

clean:
	-$(DEL) *.obj
	-$(DEL) libapi.a
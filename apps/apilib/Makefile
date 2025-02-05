TOOLPATH=../../../tools/
include ../../config.txt

OBJS_API =	api001.obj api002.obj api003.obj api004.obj api005.obj api006.obj api007.obj api008.obj api009.obj api010.obj

%.obj:%.asm
	$(NASM) -f $(NASM_FORMAT) $*.asm -o $*.obj

libapi.a:$(OBJS_API)
	$(AR) rv libapi.a $(OBJS_API)

clean:
	-$(DEL) $(OBJS_API)
	-$(DEL) libapi.a
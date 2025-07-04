TOOLPATH=../../../tools/
include ../../config.txt

OBJS_API =	napi000.obj napi001.obj napi002.obj napi003.obj napi004.obj napi005.obj napi006.obj napi007.obj \
	napi008.obj napi009.obj napi010.obj napi011.obj

%.obj:%.asm
	$(NASM) -f $(NASM_FORMAT) $*.asm -o $*.obj

libapi.a:$(OBJS_API)
	$(AR) rv libnapi.a $(OBJS_API)

clean:
	-$(DEL) $(OBJS_API)
	-$(DEL) libnapi.a
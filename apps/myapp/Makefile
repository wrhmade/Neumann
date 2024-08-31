TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(NASM) -f $(NASM_FORMAT) myapp.asm -o myapp.obj
	$(LD) myapp.obj ../apilib/libapi.a -o myapp.bin -e main 
clean:
	-$(DEL) myapp.bin
	-$(DEL) myapp.obj

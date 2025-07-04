TOOLPATH=../../../tools/
include ../../config.txt

default:
	$(GCC) -c myapp2.c -o myapp2.obj -I../include -fno-pic
	$(LD) myapp2.obj ../napilib/libnapi.a -o myapp2.elf -e main -s -Ttext 0x00
	objcopy -I elf32-i386 -O binary myapp2.elf myapp2.bin -R .note -S
clean:
	-$(DEL) myapp2.bin
	-$(DEL) myapp2.obj

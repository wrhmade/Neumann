TOOLPATH=../../../tools/
include ../../config.txt

FONTNAME = font.bin
COFFNAME = $(patsubst %.bin,%.coff,$(FONTNAME))
OBJNAME	 = $(patsubst %.bin,%.obj,$(FONTNAME))
VALUENAME = asciifnt

default:
	$(BIN2OBJ) $(FONTNAME) $(COFFNAME) $(VALUENAME)
	$(OBJCOPY) -I pe-i386 -O elf32-i386 $(COFFNAME) $(OBJNAME)

clean:
	-$(DEL) $(COFFNAME)
	-$(DEL) $(OBJNAME)
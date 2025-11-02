TOOLPATH=../../../tools/
include ../../config.txt

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
CFLAGS = -static-libgcc -g
TARGET = klotski.bin

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(OBJ) ../start/start.obj ../lib/libn.a ../napilib/libnapi.a -o $(TARGET) -e _start -s -Ttext 0x0

%.o: %.c
	$(GCC) -c $< -o $@ -I../include -fno-pic

clean:
	-$(DEL) $(OBJ) $(TARGET)
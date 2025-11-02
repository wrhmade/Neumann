TOOLPATH=../../../../tools/
include ../../../config.txt

default:
	$(GCC) -I../../include -I../../../include -c acpi.c -o acpi.obj
	$(GCC) -I../../include -I../../../include -c hpet.c -o hpet.obj

clean:
	-$(DEL) acpi.obj hpet.obj
TOOLPATH=../../tools/
include ../config.txt

default:
	$(MAKE) -C napilib
	$(MAKE) -C lib
	$(MAKE) -C start
	$(MAKE) -C myapp

clean:
	$(MAKE) -C napilib clean
	$(MAKE) -C lib clean
	$(MAKE) -C start clean
	$(MAKE) -C myapp clean
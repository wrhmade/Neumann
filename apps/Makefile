TOOLPATH=../../tools/
include ../config.txt

default:
	$(MAKE) -C apilib
	$(MAKE) -C myapp
	$(MAKE) -C myapp2

clean:
	$(MAKE) -C apilib clean
	$(MAKE) -C myapp clean
	$(MAKE) -C myapp2 clean
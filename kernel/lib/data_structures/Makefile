TOOLPATH=../../../../tools/
include ../../../config.txt

default:
	$(GCC) -I../../include ../../../include -c slist-strptr.c -o slist-strptr.obj
	$(GCC) -I../../include ../../../include -c rbtree-strptr.c -o rbtree-strptr.obj

clean:
	-$(DEL) slist-strptr.obj
	-$(DEL) rbtree-strptr.obj

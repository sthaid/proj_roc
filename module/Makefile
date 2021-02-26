EXTRA_CFLAGS += -g

KERNEL_ROOT=/lib/modules/$(shell uname -r)/build

roc_module-y := roc.o 

obj-m := roc_module.o

default:
	make -C $(KERNEL_ROOT) M=`pwd`

install:
	sudo make -C $(KERNEL_ROOT) M=`pwd` modules_install
	sudo depmod

clean:
	make -C $(KERNEL_ROOT) M=`pwd` clean

load:
	sudo modprobe roc_module
	
unload:
	sudo rmmod roc_module

reload:
	-sudo rmmod roc_module
	sudo modprobe roc_module

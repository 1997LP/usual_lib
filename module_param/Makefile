obj-m += hello_world_module.o
KDIR = /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR)  M=$(shell pwd) modules

clean:
	$(MAKE) -C $(KDIR)  M=$(shell pwd) clean

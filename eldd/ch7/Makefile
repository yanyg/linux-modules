ifneq ($(KERNELRELEASE),)
	obj-m := vms.o
coord: coord.c
	$(MAKE) -Wall -Werror -o coord coord.c
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	@rm -fr *.o *.ko *.mod.* modules.* Module.* .tmp_versions .*.cmd

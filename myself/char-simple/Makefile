ifneq ($(KERNELRELEASE),)
	obj-m := char-simple.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	@rm -fr *.o *.ko *.mod.* modules.* Module.* .tmp_versions .*.cmd

obj-m += mychar.o

KDIR ?= /home/lyx/wsl2-kernel-6.6
ifeq ($(wildcard $(KDIR)/Makefile),)
KDIR := /lib/modules/$(shell uname -r)/build
endif

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

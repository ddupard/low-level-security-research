obj-m += my_attack.o
my_attack-y := cf_attack.o attack.o
ccflags-y += -g -Og -fno-inline


KDIR := /usr/src/linux
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
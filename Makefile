obj-m += adaptive_dsp.o
adaptive_dsp-objs := dsp_core.o dsp_procfs.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	gcc -o extended_dsp_test extended_dsp_test.c -lm

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f extended_dsp_test

load:
	sudo insmod adaptive_dsp.ko

unload:
	sudo rmmod adaptive_dsp

test: load
	./extended_dsp_test

.PHONY: all clean load unload test

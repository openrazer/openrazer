KERNELDIR := /lib/modules/$(shell uname -r)/build
DRIVERDIR := $(shell pwd)/driver

all:
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) all

clean:
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) clean


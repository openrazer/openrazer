KERNELDIR := /lib/modules/$(shell uname -r)/build
DRIVERDIR := $(shell pwd)/driver

all:
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules

clean:
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) clean


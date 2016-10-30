# DESTDIR is used to install into a different root directory
DESTDIR?=/
# Specify the kernel directory to use
KERNELDIR?=/lib/modules/$(shell uname -r)/build
# Need the absolute directory do the driver directory to build kernel modules
DRIVERDIR?=$(shell pwd)/driver

# Where kernel drivers are going to be installed
MODULEDIR?=/lib/modules/$(shell uname -r)/kernel/drivers/usb/misc

# Python dir
PYTHONDIR?=/usr/lib/python3/dist-packages



# Build all target
all: driver

lp_all: lp_driver

# Driver compilation
driver:
	@echo "\n::\033[32m Compiling Razer kernel modules\033[0m"
	@echo "========================================"
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules > /dev/null 2>&1

driver_verbose:
	@echo "\n::\033[32m Compiling Razer kernel modules\033[0m"
	@echo "========================================"
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules

driver_clean:
	@echo "\n::\033[32m Cleaning Razer kernel modules\033[0m"
	@echo "========================================"
	make -C "$(KERNELDIR)" SUBDIRS="$(DRIVERDIR)" clean

# Install kernel modules and then update module dependencies
driver_install:
	@echo "\n::\033[34m Installing Razer kernel modules\033[0m"
	@echo "====================================================="
	@cp -v $(DRIVERDIR)/razerkbd.ko $(DESTDIR)/$(MODULEDIR)
	@cp -v $(DRIVERDIR)/razermouse.ko $(DESTDIR)/$(MODULEDIR)
	@cp -v $(DRIVERDIR)/razerfirefly.ko $(DESTDIR)/$(MODULEDIR)
	@chown -v root:root $(MODULEDIR)/*.ko
	depmod

# Remove kernel modules
driver_uninstall:
	@echo "\n::\033[34m Uninstalling Razer kernel modules\033[0m"
	@echo "====================================================="
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razerkbd.ko
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razermouse.ko
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razerfirefly.ko


# Launchpad hacks
lp_driver:
	@echo "\n::\033[32m Compiling Razer kernel modules (Launchpad)\033[0m"
	@echo "========================================"
	$(eval KERNELDIR:=/lib/modules/$(shell dpkg --get-selections | grep -P 'linux-headers-.+generic' | awk '{print $$1}' | tail -1 | sed 's/linux-headers-//g')/build)
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules > /dev/null 2>&1

lp_driver_clean:
	@echo "\n::\033[32m Cleaning Razer kernel modules (Launchpad)\033[0m"
	@echo "========================================"
	$(eval KERNELDIR:=/lib/modules/$(shell dpkg --get-selections | grep -P 'linux-headers-.+generic' | awk '{print $$1}' | tail -1 | sed 's/linux-headers-//g')/build)
	make -C "$(KERNELDIR)" SUBDIRS="$(DRIVERDIR)" clean


# Razer Daemon
daemon_install:
	@echo "\n::\033[34m Installing Razer Daemon\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon install
	@mkdir -p $(DESTDIR)/etc/xdg/autostart
	@cp -v ./install_files/desktop/razer-service.desktop $(DESTDIR)/etc/xdg/autostart/razer-service.desktop

daemon_uninstall:
	@echo "\n::\033[34m Uninstalling Razer Daemon\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon uninstall
	@rm -fv $(DESTDIR)/etc/xdg/autostart/razer-service.desktop


# Python Library
python_library_install:
	@echo "\n::\033[34m Installing Razer python library\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C pylib install

python_library_uninstall:
	@echo "\n::\033[34m Uninstalling Razer python library\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C pylib uninstall

# Clean target
clean: driver_clean

setup_dkms:
	@echo "\n::\033[34m Installing DKMS files\033[0m"
	@echo "====================================================="
	install -m 644 -v -D Makefile $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/Makefile
	install -m 644 -v -D install_files/dkms/dkms.conf $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/dkms.conf
	install -m 755 -v -d driver $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver
	install -m 644 -v -D driver/Makefile $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/Makefile
	install -m 644 -v driver/*.c $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/
	install -m 644 -v driver/*.h $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/
	rm -fv $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/*.mod.c

udev_install:
	@echo "\n::\033[34m Installing Razer udev rules\033[0m"
	@echo "====================================================="
	install -m 644 -v -D install_files/udev/99-razer.rules $(DESTDIR)/lib/udev/rules.d/99-razer.rules
	install -m 755 -v -D install_files/udev/razer_mount $(DESTDIR)/lib/udev/razer_mount

udev_uninstall:
	@echo "\n::\033[34m Uninstalling Razer udev rules\033[0m"
	@echo "====================================================="
	rm -f $(DESTDIR)/lib/udev/rules.d/99-razer.rules


# Install for Ubuntu
ubuntu_install: setup_dkms udev_install daemon_install python_library_install
	@echo "\n::\033[34m Installing for Ubuntu\033[0m"
	@echo "====================================================="

fedora_install:
	@echo "\n::\033[31m NOT SUPPORTED YET\033[0m"
	@echo "====================================================="



install: all driver_install udev_install python_library_install
	@make --no-print-directory -C daemon install DESTDIR=$(DESTDIR)

uninstall: driver_uninstall udev_uninstall python_library_uninstall remove_rcd_links
	@make --no-print-directory -C daemon uninstall DESTDIR=$(DESTDIR)


.PHONY: driver daemon

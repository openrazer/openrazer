# DESTDIR is used to install into a different root directory
DESTDIR?=/
# Specify the kernel directory to use
KERNELDIR?=/lib/modules/$(shell uname -r)/build
# Need the absolute directory do the driver directory to build kernel modules
DRIVERDIR?=$(shell pwd)/driver

# Where kernel drivers are going to be installed
MODULEDIR?=/lib/modules/$(shell uname -r)/kernel/drivers/hid

# Python dir
PYTHONDIR?=$(shell python3 -c 'import sys; print(sys.path[-1])')



# Build all target
all: driver

lp_all: lp_driver

# Driver compilation
driver:
	@echo -e "\n::\033[32m Compiling Razer kernel modules\033[0m"
	@echo "========================================"
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules > /dev/null 2>&1

driver_verbose:
	@echo -e "\n::\033[32m Compiling Razer kernel modules\033[0m"
	@echo "========================================"
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules

driver_clean:
	@echo -e "\n::\033[32m Cleaning Razer kernel modules\033[0m"
	@echo "========================================"
	make -C "$(KERNELDIR)" SUBDIRS="$(DRIVERDIR)" clean

# Install kernel modules and then update module dependencies
driver_install:
	@echo -e "\n::\033[34m Installing Razer kernel modules\033[0m"
	@echo "====================================================="
	@cp -v $(DRIVERDIR)/razerkbd.ko $(DESTDIR)/$(MODULEDIR)
	@cp -v $(DRIVERDIR)/razermouse.ko $(DESTDIR)/$(MODULEDIR)
	@cp -v $(DRIVERDIR)/razerfirefly.ko $(DESTDIR)/$(MODULEDIR)
	@chown -v root:root $(MODULEDIR)/*.ko
	depmod

# Remove kernel modules
driver_uninstall:
	@echo -e "\n::\033[34m Uninstalling Razer kernel modules\033[0m"
	@echo "====================================================="
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razerkbd.ko
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razermouse.ko
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razerfirefly.ko


# Launchpad hacks
lp_driver:
	@echo -e "\n::\033[32m Compiling Razer kernel modules (Launchpad)\033[0m"
	@echo "========================================"
	$(eval KERNELDIR:=/lib/modules/$(shell dpkg --get-selections | grep -P 'linux-headers-.+generic' | awk '{print $$1}' | tail -1 | sed 's/linux-headers-//g')/build)
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules > /dev/null 2>&1

lp_driver_clean:
	@echo -e "\n::\033[32m Cleaning Razer kernel modules (Launchpad)\033[0m"
	@echo "========================================"
	$(eval KERNELDIR:=/lib/modules/$(shell dpkg --get-selections | grep -P 'linux-headers-.+generic' | awk '{print $$1}' | tail -1 | sed 's/linux-headers-//g')/build)
	make -C "$(KERNELDIR)" SUBDIRS="$(DRIVERDIR)" clean


# Razer Daemon
daemon_install:
	@echo -e "\n::\033[34m Installing Razer Daemon\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon install
	@mkdir -p $(DESTDIR)/etc/xdg/autostart
	@cp -v ./install_files/desktop/razer-service.desktop $(DESTDIR)/etc/xdg/autostart/razer-service.desktop

fedora_daemon_install:
	@echo "\n::\033[34m Installing Razer Daemon (Fedora)\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon fedora_install
	@mkdir -p $(DESTDIR)/etc/xdg/autostart
	@cp -v ./install_files/desktop/razer-service.desktop $(DESTDIR)/etc/xdg/autostart/razer-service.desktop

daemon_uninstall:
	@echo -e "\n::\033[34m Uninstalling Razer Daemon\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon uninstall
	@rm -fv $(DESTDIR)/etc/xdg/autostart/razer-service.desktop


# Python Library
python_library_install:
	@echo -e "\n::\033[34m Installing Razer python library\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C pylib install

python_library_uninstall:
	@echo -e "\n::\033[34m Uninstalling Razer python library\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C pylib uninstall

# Clean target
clean: driver_clean

setup_dkms:
	@echo -e "\n::\033[34m Installing DKMS files\033[0m"
	@echo "====================================================="
	install -m 644 -v -D Makefile $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/Makefile
	install -m 644 -v -D install_files/dkms/dkms.conf $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/dkms.conf
	install -m 755 -v -d driver $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver
	install -m 644 -v -D driver/Makefile $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/Makefile
	install -m 644 -v driver/*.c $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/
	install -m 644 -v driver/*.h $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/
	rm -fv $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/*.mod.c

remove_dkms:
    @echo "\n::\033[34m Removing DKMS files\033[0m"
	@echo "====================================================="
	rm -rf $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0

udev_install:
	@echo -e "\n::\033[34m Installing Razer udev rules\033[0m"
	@echo "====================================================="
	install -m 644 -v -D install_files/udev/99-razer.rules $(DESTDIR)/lib/udev/rules.d/99-razer.rules
	install -m 755 -v -D install_files/udev/razer_mount $(DESTDIR)/lib/udev/razer_mount

udev_uninstall:
	@echo -e "\n::\033[34m Uninstalling Razer udev rules\033[0m"
	@echo "====================================================="
	rm -f $(DESTDIR)/lib/udev/rules.d/99-razer.rules $(DESTDIR)/lib/udev/razer_mount

# Install for Ubuntu
ubuntu_install: setup_dkms udev_install daemon_install python_library_install
	@echo -e "\n::\033[34m Installing for Ubuntu\033[0m"
	@echo "====================================================="

fedora_install: setup_dkms udev_install
	@echo -e "\n::\033[34m Installing for Fedora\033[0m"
	@echo "====================================================="
	$(eval PYTHONDIR:=/usr/lib/python3.5/site-packages)
	@make fedora_daemon_install DESTDIR=$(DESTDIR) PYTHONDIR=$(PYTHONDIR)
	@make python_library_install DESTDIR=$(DESTDIR) PYTHONDIR=$(PYTHONDIR)

fedora_uninstall: remove_dkms udev_uninstall
	@echo -e "\n::\033[34m Installing for Fedora\033[0m"
	@echo "====================================================="
	$(eval PYTHONDIR:=/usr/lib/python3.5/site-packages)
	@make daemon_uninstall DESTDIR=$(DESTDIR)
	@make python_library_uninstall DESTDIR=$(DESTDIR) PYTHONDIR=$(PYTHONDIR)

fedora_package:
	$(eval RZRTMPDIR:=$(shell mktemp -d))
	$(eval VERSION:=$(shell cat debian/changelog | grep -Po '([0-9]+\.?)+' | head -1))
	@make fedora_install DESTDIR:=$(RZRTMPDIR)
	@echo -e "\n::\033[34m Making fedora package\033[0m"
	@echo "====================================================="
	mkdir -p dist
	fpm --force -s dir -t rpm -d kernel-devel -d dkms -d udev -n razer-kernel-modules-dkms -v $(VERSION) -p dist/ -a all --before-install install_files/fedora_package_scripts/razer-kernel-modules-dkms.preinst --after-install install_files/fedora_package_scripts/razer-kernel-modules-dkms.postinst --before-remove install_files/fedora_package_scripts/razer-kernel-modules-dkms.prerm -m "Terry Cain <terry+razer@terrys-home.co.uk>" --url "https://github.com/terrycain/razer-drivers" --description "Razer Driver DKMS package"  -C $(RZRTMPDIR) lib usr/src
	fpm --force -s dir -t rpm -d razer-kernel-modules-dkms -d python3 -d python3-dbus -d python3-gobject -d python3-setproctitle -d xautomation -d xdotool -n razer-daemon -p dist/ -v $(VERSION) -a all --before-remove install_files/fedora_package_scripts/razer-daemon.prerm -m "Terry Cain <terry+razer@terrys-home.co.uk>" --url "https://github.com/terrycain/razer-drivers" --description "Razer Service package"  -C $(RZRTMPDIR) usr/lib/python3.5/site-packages/razer_daemon usr/bin/razer-service etc/xdg/autostart usr/share/razer-service usr/share/man
	fpm --force -s dir -t rpm -d python3 -d python3-dbus -d python3-gobject -d python3-numpy -n python3-razer -p dist/ -v $(VERSION) -a all --before-remove install_files/fedora_package_scripts/python3-razer.prerm -m "Terry Cain <terry+razer@terrys-home.co.uk>" --url "https://github.com/terrycain/razer-drivers" --description "Razer Python library"  -C $(RZRTMPDIR) usr/lib/python3.5/site-packages/razer usr/bin/razer-service
	rm -rf $(RZRTMPDIR)


install: all driver_install udev_install python_library_install
	@make --no-print-directory -C daemon install DESTDIR=$(DESTDIR)

uninstall: driver_uninstall udev_uninstall python_library_uninstall
	@make --no-print-directory -C daemon uninstall DESTDIR=$(DESTDIR)


.PHONY: driver daemon

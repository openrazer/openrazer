# DESTDIR is used to install into a different root directory
DESTDIR?=/
# Specify the kernel directory to use
KERNELDIR?=/lib/modules/$(shell uname -r)/build
# Need the absolute directory do the driver directory to build kernel modules
DRIVERDIR?=$(shell pwd)/driver

# Where kernel drivers are going to be installed
MODULEDIR?=/lib/modules/$(shell uname -r)/kernel/drivers/usb/misc

# Python dir
PYTHONDIR?=/usr/lib/python3.4/site-packages

# Build all target
all: librazer_chroma daemon daemon_controller examples driver

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
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) clean > /dev/null 2>&1

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
	@cp -v $(DESTDIR)/$(MODULEDIR)/razerkbd.ko
	@cp -v $(DESTDIR)/$(MODULEDIR)/razermouse.ko
	@cp -v $(DESTDIR)/$(MODULEDIR)/razerfirefly.ko


# Razer Chroma Library
librazer_chroma:
	make --no-print-directory -C lib all

librazer_chroma_clean:
	make --no-print-directory -C lib clean

# Razer Daemon
daemon:
	make --no-print-directory -C daemon all

daemon_clean:
	make --no-print-directory -C daemon clean

# Razer Daemon contoller
daemon_controller:
	make --no-print-directory -C daemon_controller all

daemon_controller_clean:
	make --no-print-directory -C daemon_controller clean

# Examples
examples:
	make --no-print-directory -C examples all

examples_clean:
	make --no-print-directory -C examples clean



# Python Library
python_library_install:
	@echo "\n::\033[34m Installing Razer python library\033[0m"
	@echo "====================================================="
	@install -v -d pylib $(DESTDIR)/$(PYTHONDIR)/razer
	@cp -v -r pylib/* $(DESTDIR)/$(PYTHONDIR)/razer

python_library_uninstall:
	@echo "\n::\033[34m Uninstalling Razer python library\033[0m"
	@echo "====================================================="
	rm -rf $(DESTDIR)/$(PYTHONDIR)/razer

# Clean target
clean: librazer_chroma_clean daemon_clean daemon_controller_clean examples_clean driver_clean

# Build all target
all: librazer_chroma daemon daemon_controller examples driver

setup_dkms: driver
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
	install -m 644 -v -D install_files/udev/95-razerkbd.rules $(DESTDIR)/lib/udev/rules.d/95-razerkbd.rules

udev_uninstall:
	@echo "\n::\033[34m Uninstalling Razer udev rules\033[0m"
	@echo "====================================================="
	rm -f $(DESTDIR)/lib/udev/rules.d/95-razerkbd.rules

dbus_install:
	@echo "\n::\033[34m Installing Razer chroma DBus policy\033[0m"
	@echo "====================================================="
	install -m 644 -v -D install_files/dbus/org.voyagerproject.razer.daemon.conf $(DESTDIR)/etc/dbus-1/system.d/org.voyagerproject.razer.daemon.conf

dbus_uninstall:
	@echo "\n::\033[34m Uninstalling Razer chroma DBus policy\033[0m"
	@echo "====================================================="
	rm -f $(DESTDIR)/etc/dbus-1/system.d/org.voyagerproject.razer.daemon.conf

rcd_links:
	mkdir -p $(DESTDIR)/etc/rc0.d $(DESTDIR)/etc/rc1.d $(DESTDIR)/etc/rc2.d $(DESTDIR)/etc/rc3.d $(DESTDIR)/etc/rc4.d $(DESTDIR)/etc/rc5.d $(DESTDIR)/etc/rc6.d
	ln -fs ../init.d/razer_bcd $(DESTDIR)/etc/rc2.d/S24razer_bcd
	ln -fs ../init.d/razer_bcd $(DESTDIR)/etc/rc3.d/S24razer_bcd
	ln -fs ../init.d/razer_bcd $(DESTDIR)/etc/rc4.d/S24razer_bcd
	ln -fs ../init.d/razer_bcd $(DESTDIR)/etc/rc5.d/S24razer_bcd
	ln -fs ../init.d/razer_bcd $(DESTDIR)/etc/rc0.d/K02razer_bcd
	ln -fs ../init.d/razer_bcd $(DESTDIR)/etc/rc1.d/K02razer_bcd
	ln -fs ../init.d/razer_bcd $(DESTDIR)/etc/rc6.d/K02razer_bcd

remove_rcd_links:
	rm -f $(DESTDIR)/etc/rc2.d/S24razer_bcd
	rm -f $(DESTDIR)/etc/rc3.d/S24razer_bcd
	rm -f $(DESTDIR)/etc/rc4.d/S24razer_bcd
	rm -f $(DESTDIR)/etc/rc5.d/S24razer_bcd
	rm -f $(DESTDIR)/etc/rc0.d/K02razer_bcd
	rm -f $(DESTDIR)/etc/rc1.d/K02razer_bcd
	rm -f $(DESTDIR)/etc/rc6.d/K02razer_bcd

# Install for Ubuntu
ubuntu_install: all setup_dkms udev_install dbus_install
	@echo "\n::\033[34m Installing for Ubuntu\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C lib install DESTDIR=$(DESTDIR)
	@make --no-print-directory -C daemon install DESTDIR=$(DESTDIR)
	@make --no-print-directory -C daemon_controller install DESTDIR=$(DESTDIR)
	
	@make --no-print-directory python_library_install PYTHONDIR=/usr/lib/python3/dist-packages

# Ubuntu 14.04 uses upstart
ubuntu_14_04_install ubuntu_14_10_install: ubuntu_install
	@echo "\n::\033[34m Installing Razer daemon upstart file\033[0m"
	@echo "====================================================="
	@install -m 644 -v -D install_files/init/razer_bcd.conf $(DESTDIR)/etc/init/razer_bcd.conf
	@install -m 755 -v -D install_files/init.d/razer_bcd_ubuntu $(DESTDIR)/etc/init.d/razer_bcd
	@install -m 755 -v -D install_files/share/bash_keyboard_functions.sh $(DESTDIR)/usr/share/razer_bcd/bash_keyboard_functions.sh
	
	@make --no-print-directory rcd_links

# Ubuntu 15.04+ uses systemd
ubuntu_15_04_install ubuntu_15_10_install ubuntu_16_04_install: ubuntu_install
	@echo "\n::\033[34m Installing Razer daemon systemd file\033[0m"
	@echo "====================================================="
	@install -v -D install_files/systemd/razer_bcd.service $(DESTDIR)/lib/systemd/system/razer_bcd.service
	@install -v -D install_files/init.d/razer_bcd_ubuntu $(DESTDIR)/etc/init.d/razer_bcd
	@install -v -D install_files/share/bash_keyboard_functions.sh $(DESTDIR)/usr/share/razer_bcd/bash_keyboard_functions.sh
	@install -v -D install_files/share/systemd_helpers.sh $(DESTDIR)/usr/share/razer_bcd/systemd_helpers.sh
	
	@make --no-print-directory rcd_links

fedora_install:
	@echo "\n::\033[34m Installing for Fedora 23\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C lib install DESTDIR=$(DESTDIR) LIBDIR=/usr/lib64
	@make --no-print-directory -C daemon install DESTDIR=$(DESTDIR)
	@make --no-print-directory -C daemon_controller install DESTDIR=$(DESTDIR)
	
	@make --no-print-directory python_library_install PYTHONDIR=/usr/lib/python3/dist-packages
	
	@echo "\n::\033[34m Installing Razer systemd daemon file\033[0m"
	@echo "====================================================="
	@install -v -D install_files/systemd/razer_bcd.service $(DESTDIR)/lib/systemd/system/razer_bcd.service
	@install -v -D install_files/init.d/razer_bcd_ubuntu $(DESTDIR)/etc/init.d/razer_bcd
	@install -v -D install_files/share/bash_keyboard_functions.sh $(DESTDIR)/usr/share/razer_bcd/bash_keyboard_functions.sh
	@install -v -D install_files/share/systemd_helpers.sh $(DESTDIR)/usr/share/razer_bcd/systemd_helpers.sh



install: all driver_install udev_install dbus_install python_library_install
	@make --no-print-directory -C lib install DESTDIR=$(DESTDIR)
	@make --no-print-directory -C daemon install DESTDIR=$(DESTDIR)
	@make --no-print-directory -C daemon_controller install DESTDIR=$(DESTDIR)
	
	@echo "\n::\033[34m Installing Razer daemon init.d file\033[0m"
	@echo "====================================================="
	@install -v -D install_files/init.d/razer_bcd $(DESTDIR)/etc/init.d/razer_bcd
	@install -v -D install_files/share/bash_keyboard_functions.sh $(DESTDIR)/usr/share/razer_bcd/bash_keyboard_functions.sh
	
	@make --no-print-directory rcd_links

uninstall: driver_uninstall udev_uninstall dbus_uninstall python_library_uninstall remove_rcd_links
	@make --no-print-directory -C lib uninstall DESTDIR=$(DESTDIR)
	@make --no-print-directory -C daemon uninstall DESTDIR=$(DESTDIR)
	@make --no-print-directory -C daemon_controller uninstall DESTDIR=$(DESTDIR)
	
	@rm -f $(DESTDIR)/lib/systemd/system/razer_bcd.service
	@rm -f $(DESTDIR)/etc/init.d/razer_bcd
	@rm -f $(DESTDIR)/etc/init/razer_bcd.conf
	@rm -r $(DESTDIR)/usr/share/razer_bcd/bash_keyboard_functions.sh
	@rm -f $(DESTDIR)/usr/share/razer_bcd/systemd_helpers.sh


.PHONY: driver daemon daemon_controller examples

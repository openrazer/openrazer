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

DKMS_NAME?=openrazer-driver
DKMS_VER?=2.4.0


# Build all target
all: driver

# Driver compilation
driver:
	@echo -e "\n::\033[32m Compiling OpenRazer kernel modules\033[0m"
	@echo "========================================"
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules

driver_clean:
	@echo -e "\n::\033[32m Cleaning OpenRazer kernel modules\033[0m"
	@echo "========================================"
	make -C "$(KERNELDIR)" SUBDIRS="$(DRIVERDIR)" clean

# Install kernel modules and then update module dependencies
driver_install:
	@echo -e "\n::\033[34m Installing OpenRazer kernel modules\033[0m"
	@echo "====================================================="
	@cp -v $(DRIVERDIR)/*.ko $(DESTDIR)/$(MODULEDIR)
	@chown -v root:root $(DESTDIR)/$(MODULEDIR)/*.ko
	depmod

# Just use for packaging openrazer, not for installing manually
driver_install_packaging:
	@echo -e "\n::\033[34m Installing OpenRazer kernel modules\033[0m"
	@echo "====================================================="
	@cp -v $(DRIVERDIR)/*.ko $(DESTDIR)/$(MODULEDIR)

# Remove kernel modules
driver_uninstall:
	@echo -e "\n::\033[34m Uninstalling OpenRazer kernel modules\033[0m"
	@echo "====================================================="
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razerkbd.ko
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razermouse.ko
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razerfirefly.ko
	@rm -fv $(DESTDIR)/$(MODULEDIR)/razercore.ko

# Razer Daemon
daemon_install:
	@echo -e "\n::\033[34m Installing OpenRazer Daemon\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon install

ubuntu_daemon_install:
	@echo -e "\n::\033[34m Installing OpenRazer Daemon\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon ubuntu_install


daemon_uninstall:
	@echo -e "\n::\033[34m Uninstalling OpenRazer Daemon\033[0m"
	@echo "====================================================="
	make --no-print-directory -C daemon uninstall

# Python Library
python_library_install:
	@echo -e "\n::\033[34m Installing OpenRazer python library\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C pylib install

ubuntu_python_library_install:
	@echo -e "\n::\033[34m Installing OpenRazer python library\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C pylib ubuntu_install

python_library_uninstall:
	@echo -e "\n::\033[34m Uninstalling OpenRazer python library\033[0m"
	@echo "====================================================="
	@make --no-print-directory -C pylib uninstall

# Legacy XDG autostart
xdg_install:
	@mkdir -p $(DESTDIR)/etc/xdg/autostart
	@cp -v ./install_files/desktop/openrazer-daemon.desktop $(DESTDIR)/etc/xdg/autostart/openrazer-daemon.desktop

xdg_uninstall:
	@rm -fv $(DESTDIR)/etc/xdg/autostart/openrazer-daemon.desktop

install-systemd:
	@make --no-print-directory -C daemon install-systemd

# Clean target
clean: driver_clean

setup_dkms:
	@echo -e "\n::\033[34m Installing DKMS files\033[0m"
	@echo "====================================================="
	install -m 644 -v -D Makefile $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)/Makefile
	install -m 644 -v -D install_files/dkms/dkms.conf $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)/dkms.conf
	install -m 755 -v -d driver $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)/driver
	install -m 644 -v -D driver/Makefile $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)/driver/Makefile
	install -m 644 -v driver/*.c $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)/driver/
	install -m 644 -v driver/*.h $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)/driver/
	rm -fv $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)/driver/*.mod.c

remove_dkms:
	@echo "\n::\033[34m Removing DKMS files\033[0m"
	@echo "====================================================="
	rm -rf $(DESTDIR)/usr/src/$(DKMS_NAME)-$(DKMS_VER)

udev_install:
	@echo -e "\n::\033[34m Installing OpenRazer udev rules\033[0m"
	@echo "====================================================="
	install -m 644 -v -D install_files/udev/99-razer.rules $(DESTDIR)/usr/lib/udev/rules.d/99-razer.rules
	install -m 755 -v -D install_files/udev/razer_mount $(DESTDIR)/usr/lib/udev/razer_mount

udev_uninstall:
	@echo -e "\n::\033[34m Uninstalling OpenRazer udev rules\033[0m"
	@echo "====================================================="
	rm -f $(DESTDIR)/usr/lib/udev/rules.d/99-razer.rules $(DESTDIR)/usr/lib/udev/razer_mount

ubuntu_udev_install:
	@echo -e "\n::\033[34m Installing OpenRazer udev rules\033[0m"
	@echo "====================================================="
	install -m 644 -v -D install_files/udev/99-razer.rules $(DESTDIR)/lib/udev/rules.d/99-razer.rules
	install -m 755 -v -D install_files/udev/razer_mount $(DESTDIR)/lib/udev/razer_mount

ubuntu_udev_uninstall:
	@echo -e "\n::\033[34m Uninstalling OpenRazer udev rules\033[0m"
	@echo "====================================================="
	rm -f $(DESTDIR)/lib/udev/rules.d/99-razer.rules $(DESTDIR)/lib/udev/razer_mount

appstream_install:
	@echo -e "\n::\033[34m Installing OpenRazer AppStream metadata\033[0m"
	@echo "====================================================="
	install -m 644 -v -D install_files/appstream/io.github.openrazer.openrazer.metainfo.xml $(DESTDIR)/usr/share/metainfo/io.github.openrazer.openrazer.metainfo.xml

appstream_uninstall:
	@echo -e "\n::\033[34m Uninstalling OpenRazer AppStream metadata\033[0m"
	@echo "====================================================="
	rm -f $(DESTDIR)/usr/share/metainfo/io.github.openrazer.openrazer.metainfo.xml

# Install for Ubuntu
ubuntu_install: setup_dkms ubuntu_udev_install ubuntu_daemon_install ubuntu_python_library_install appstream_install
	@echo -e "\n::\033[34m Installing for Ubuntu\033[0m"
	@echo "====================================================="
	mv $(DESTDIR)/usr/lib/python3.* $(DESTDIR)/usr/lib/python3
	mv $(DESTDIR)/usr/lib/python3/site-packages $(DESTDIR)/usr/lib/python3/dist-packages

install_i_know_what_i_am_doing: all driver_install udev_install python_library_install
	@make --no-print-directory -C daemon install DESTDIR=$(DESTDIR)

install: manual_install_msg ;

manual_install_msg:
	@echo "Please do not install the driver using this method. Use a distribution package as it tracks the files installed and can remove them afterwards. If you are 100% sure, you want to do this, find the correct target in the Makefile."
	@echo "Exiting."

uninstall: driver_uninstall udev_uninstall python_library_uninstall
	@make --no-print-directory -C daemon uninstall DESTDIR=$(DESTDIR)


.PHONY: driver

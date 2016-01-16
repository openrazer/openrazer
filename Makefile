KERNELDIR?=/lib/modules/$(shell uname -r)/build
DESTDIR?=/
DRIVERDIR:=$(shell pwd)/driver
MODULEDIR=/lib/modules/$(shell uname -r)/kernel/drivers/usb/misc

all: librazer_chroma razer_daemon razer_daemon_controller razer_examples razer_drv

# added redirect to remove output of a useless makefile warning
razer_drv:
	@echo "\n::\033[32m COMPILING razer kernel modules\033[0m"
	@echo "========================================"
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules > /dev/null 2>&1

razer_drv_verbose:
	@echo "\n::\033[32m COMPILING razer kernel modules\033[0m"
	@echo "========================================"
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) modules


librazer_chroma: 
	make -C lib all

librazer_chroma_clean: 
	make -C lib clean 

razer_daemon: 
	make -C daemon all

razer_daemon_clean: 
	make -C daemon clean 

razer_daemon_controller: 
	make -C daemon_controller all

razer_daemon_controller_clean: 
	make -C daemon_controller clean 

razer_examples: 
	make -C examples all

razer_examples_clean: 
	make -C examples clean 

fedora_install: all
    # Install the main binaries
	make -C lib               fedora_install DESTDIR=$(DESTDIR)
	make -C daemon            fedora_install DESTDIR=$(DESTDIR)
	make -C daemon_controller fedora_install DESTDIR=$(DESTDIR)
	
	# Install UDEV conf
	install -v -D install_files/udev/95-razerkbd.rules $(DESTDIR)/etc/udev/rules.d/95-razerkbd.rules
	
	# Install DBUS conf
	install -v -D install_files/dbus/org.voyagerproject.razer.daemon.conf $(DESTDIR)/etc/dbus-1/system.d/org.voyagerproject.razer.daemon.conf
	
	# Install bash helper functions
	install -v -D install_files/share/bash_keyboard_functions.sh $(DESTDIR)/usr/share/razer_bcd/bash_keyboard_functions.sh
	install -v -D  install_files/share/bash_keyboard_functions.sh $(DESTDIR)/usr/share/razer_bcd/systemd_helpers.sh
	
	# Copy over systemd config
	mkdir -p $(DESTDIR)/etc/systemd/system/
	install -v -D install_files/systemd/razer_bcd.service $(DESTDIR)/usr/lib/systemd/system/razer_bcd.service
	ln -s /usr/lib/systemd/system/razer_bcd.service $(DESTDIR)/etc/systemd/system/razer_bcd.service
	
	# Install application entries	
	install -v -D install_files/desktop/razer_tray_applet.desktop $(DESTDIR)/usr/share/applications/razer_tray_applet.desktop
	install -v -D install_files/desktop/razer_chroma_controller.desktop $(DESTDIR)/usr/share/applications/razer_chroma_controller.desktop
	
	# Install Python3 library
	install -v -d gui/lib/razer $(DESTDIR)/usr/lib/python3/dist-packages/razer
	cp -v -r gui/lib/razer/* $(DESTDIR)/usr/lib/python3/dist-packages/razer
	
	# Install Tray application
	install -v -d gui/tray_applet $(DESTDIR)/usr/share/razer_tray_applet
	cp -v -r gui/tray_applet/* $(DESTDIR)/usr/share/razer_tray_applet
	
	# Install Chroma App
	install -v -d gui/chroma_controller $(DESTDIR)/usr/share/razer_chroma_controller
	cp -v -r gui/chroma_controller/* $(DESTDIR)/usr/share/razer_chroma_controller
	
	# Copy razer kernel driver to src
	install -v -D Makefile $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/Makefile
	install -v -D install_files/dkms/dkms.conf $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/dkms.conf
	install -v -d driver $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver
	cp -v driver/Makefile $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/
	cp -v driver/*.c $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/
	cp -v driver/*.h $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/
	rm -fv $(DESTDIR)/usr/src/razer_chroma_driver-1.0.0/driver/*.mod.c

	

install: all
	make -C lib install
	make -C daemon install
	make -C daemon_controller install
	@echo "\n::\033[32m INSTALLING razer chroma kernel module\033[0m"
	@echo "====================================================="
	cp $(DRIVERDIR)/razerkbd.ko $(MODULEDIR)
	chown root:root $(MODULEDIR)/razerkbd.ko
	depmod
	@echo "\n::\033[32m INSTALLING razer chroma udev rules\033[0m"
	@echo "====================================================="
	cp install_files/udev/95-razerkbd.rules /etc/udev/rules.d
	chown root:root /etc/udev/rules.d/95-razerkbd.rules
	@echo "\n::\033[32m INSTALLING razer chroma dbus policy\033[0m"
	@echo "====================================================="
	cp install_files/dbus/org.voyagerproject.razer.daemon.conf /etc/dbus-1/system.d
	chown root:root /etc/dbus-1/system.d/org.voyagerproject.razer.daemon.conf
	@echo "\n::\033[32m INSTALLING razer daemon init.d file\033[0m"
	@echo "====================================================="
	cp install_files/init.d/razer_bcd /etc/init.d
	chown root:root /etc/init.d/razer_bcd
	cp install_files/share/bash_keyboard_functions.sh /usr/share/razer_bcd/bash_keyboard_functions.sh
	chown root:root /usr/share/razer_bcd/bash_keyboard_functions.sh
	ln -fs ../init.d/razer_bcd /etc/rc2.d/S24razer_bcd
	ln -fs ../init.d/razer_bcd /etc/rc3.d/S24razer_bcd
	ln -fs ../init.d/razer_bcd /etc/rc4.d/S24razer_bcd
	ln -fs ../init.d/razer_bcd /etc/rc5.d/S24razer_bcd

	ln -fs ../init.d/razer_bcd /etc/rc0.d/K02razer_bcd
	ln -fs ../init.d/razer_bcd /etc/rc1.d/K02razer_bcd
	ln -fs ../init.d/razer_bcd /etc/rc6.d/K02razer_bcd

uninstall:
	make -C lib uninstall
	make -C daemon uninstall
	rm /etc/init.d/razer_bcd
	rm /etc/udev/rules.d/95-razerkbd.rules
	rm /etc/rc2.d/S24razer_bcd
	rm /etc/rc3.d/S24razer_bcd
	rm /etc/rc4.d/S24razer_bcd
	rm /etc/rc5.d/S24razer_bcd
	rm /etc/rc0.d/K02razer_bcd
	rm /etc/rc1.d/K02razer_bcd
	rm /etc/rc6.d/K02razer_bcd
	rm /etc/dbus-1/system.d/org.voyagerproject.razer.daemon.conf
	rm /usr/sbin/razer_blackwidow_chroma_activate_driver.sh


clean: librazer_chroma_clean razer_daemon_clean razer_daemon_controller_clean razer_examples_clean razer_drv_clean

razer_drv_clean:
	make -C $(KERNELDIR) SUBDIRS=$(DRIVERDIR) clean > /dev/null 2>&1
	#added redirect to remove output of a useless makefile warning


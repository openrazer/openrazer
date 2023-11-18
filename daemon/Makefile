# SPDX-License-Identifier: GPL-2.0-or-later

DESTDIR?=/
PREFIX?=/usr

purge_pycache:
	@find -name '__pycache__' | xargs rm -rf

service:
	@sed "s|##PREFIX##|$(PREFIX)|" resources/org.razer.service.in > org.razer.service
	@sed "s|##PREFIX##|$(PREFIX)|" resources/openrazer-daemon.systemd.service.in > openrazer-daemon.service

install-resources: service
	@install -m 644 -D -v resources/razer.conf $(DESTDIR)$(PREFIX)/share/openrazer/razer.conf.example
	@install -m 755 -D -v run_openrazer_daemon.py $(DESTDIR)$(PREFIX)/bin/openrazer-daemon

install-systemd: service
	@install -m 644 -D -v org.razer.service $(DESTDIR)$(PREFIX)/share/dbus-1/services/org.razer.service
	@install -m 644 -D -v openrazer-daemon.service $(DESTDIR)$(PREFIX)/lib/systemd/user/openrazer-daemon.service

install: purge_pycache manpages install-resources install-systemd
	python3 setup.py install --prefix=$(PREFIX) --root=$(DESTDIR)

ubuntu_install: purge_pycache manpages install-resources
	python3 setup.py install --prefix=$(PREFIX) --root=$(DESTDIR) --no-compile --install-layout=deb

manpages:
	@install -m 644 -D -v resources/man/razer.conf.5 $(DESTDIR)$(PREFIX)/share/man/man5/razer.conf.5
	@gzip -n $(DESTDIR)$(PREFIX)/share/man/man5/razer.conf.5
	@install -m 644 -D -v resources/man/openrazer-daemon.8 $(DESTDIR)$(PREFIX)/share/man/man8/openrazer-daemon.8
	@gzip -n $(DESTDIR)$(PREFIX)/share/man/man8/openrazer-daemon.8

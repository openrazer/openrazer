DESTDIR?=/
PREFIX?=/usr

purge_pycache:
	@find -name '__pycache__' | xargs rm -rf

service:
	@sed "s|##PREFIX##|$(PREFIX)|" resources/org.razer.service.in > org.razer.service
	@sed "s|##PREFIX##|$(PREFIX)|" resources/razer-daemon.systemd.service.in > razer-daemon.service

install-resources: service
	@install -m 644 -D -v resources/razer.conf $(DESTDIR)$(PREFIX)/share/razer-daemon/razer.conf.example
	@install -m 755 -D -v run_razer_daemon.py $(DESTDIR)$(PREFIX)/bin/razer-daemon
	@install -m 644 -D -v org.razer.service $(DESTDIR)$(PREFIX)/share/dbus-1/services/org.razer.service
	@install -m 644 -D -v razer-daemon.service $(DESTDIR)$(PREFIX)/lib/systemd/user/razer-daemon.service

install: purge_pycache manpages install-resources
	python3 setup.py install --prefix=$(PREFIX) --root=$(DESTDIR)

ubuntu_install: purge_pycache manpages install-resources
	python3 setup.py install --prefix=$(PREFIX) --root=$(DESTDIR) --no-compile

manpages:
	@install -m 644 -D -v resources/man/razer.conf.5 $(DESTDIR)$(PREFIX)/share/man/man5/razer.conf.5
	@gzip $(DESTDIR)$(PREFIX)/share/man/man5/razer.conf.5
	@install -m 644 -D -v resources/man/razer-daemon.8 $(DESTDIR)$(PREFIX)/share/man/man8/razer-daemon.8
	@gzip $(DESTDIR)$(PREFIX)/share/man/man8/razer-daemon.8

manpages_uninstall:
	@rm -f $(DESTDIR)$(PREFIX)/share/man/man5/razer.conf.5.gz
	@rm -f $(DESTDIR)$(PREFIX)/share/man/man8/razer-daemon.8.gz

uninstall: manpages_uninstall
	$(eval DAEMONPATH := $(shell find $(DESTDIR)$(PREFIX)/lib/python3* -maxdepth 2 -name "razer_daemon"))
	$(eval EGGPATH := $(shell readlink -f $(DAEMONPATH)-*.egg-info/)) # shows an error when it's not already installed (maybe use some if condition)
	@rm -rf -v $(DAEMONPATH)
	@rm -rf -v $(EGGPATH)
	@rm -rf -v $(DESTDIR)$(PREFIX)/share/razer-daemon
	@rm -f -v $(DESTDIR)$(PREFIX)/bin/razer-daemon

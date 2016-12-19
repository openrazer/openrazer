DESTDIR?=/
PYTHONDIR?=$(shell python3 -c 'import sys; print(sys.path[-1])')

purge_pycache:
	@find -name '__pycache__' | xargs rm -rf

install: purge_pycache
	@install -v -d razer_daemon $(DESTDIR)/$(PYTHONDIR)/razer_daemon
	@cp -v -r razer_daemon/* $(DESTDIR)/$(PYTHONDIR)/razer_daemon
	@install -m 644 -D -v resources/razer.conf $(DESTDIR)/usr/share/razer-service/razer.conf.example
	@install -m 644 -D -v resources/razer-service.conf $(DESTDIR)/usr/share/upstart/sessions/razer-service.conf
	@install -m 755 -D -v run_razer_daemon.py $(DESTDIR)/usr/bin/razer-service

fedora_install: install
	@install -m 644 -D -v resources/man/razer.conf.5 $(DESTDIR)/usr/share/man/man5/razer.conf.5
	@gzip $(DESTDIR)/usr/share/man/man5/razer.conf.5
	@install -m 644 -D -v resources/man/razer-service.8 $(DESTDIR)/usr/share/man/man8/razer-service.8
	@gzip $(DESTDIR)/usr/share/man/man8/razer-service.8

uninstall:
	@rm -rf -v $(DESTDIR)/$(PYTHONDIR)/razer_daemon
	@rm -rf -v $(DESTDIR)/usr/share/razer-service
	@rm -f -v $(DESTDIR)/usr/share/upstart/sessions/razer-service.conf
	@rm -f -v $(DESTDIR)/usr/bin/razer-service

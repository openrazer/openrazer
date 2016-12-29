DESTDIR?=/

purge_pycache:
	@find -name '__pycache__' | xargs rm -rf

install: purge_pycache manpages
	python3 setup.py install --root=$(DESTDIR)
	@install -m 644 -D -v resources/razer.conf $(DESTDIR)/usr/share/razer-service/razer.conf.example
	@install -m 755 -D -v run_razer_daemon.py $(DESTDIR)/usr/bin/razer-service

manpages:
	@install -m 644 -D -v resources/man/razer.conf.5 $(DESTDIR)/usr/share/man/man5/razer.conf.5
	@gzip $(DESTDIR)/usr/share/man/man5/razer.conf.5
	@install -m 644 -D -v resources/man/razer-service.8 $(DESTDIR)/usr/share/man/man8/razer-service.8
	@gzip $(DESTDIR)/usr/share/man/man8/razer-service.8

manpages_uninstall:
	@rm -f $(DESTDIR)/usr/share/man/man5/razer.conf.5.gz
	@rm -f $(DESTDIR)/usr/share/man/man8/razer-service.8.gz

uninstall:
	$(eval DAEMONPATH := $(shell find $(DESTDIR)/usr/lib/python3* -maxdepth 2 -name "razer_daemon"))
	$(eval EGGPATH := $(shell readlink -f $(DAEMONPATH)-*.egg-info/)) # shows an error when it's not already installed (maybe use some if condition)
	@rm -rf -v $(DAEMONPATH)
	@rm -rf -v $(EGGPATH)
	@rm -rf -v $(DESTDIR)/usr/share/razer-service
	@rm -f -v $(DESTDIR)/usr/bin/razer-service

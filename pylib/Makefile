DESTDIR?=/

purge_pycache:
	@find -name '__pycache__' | xargs rm -rf

install: purge_pycache
	python3 setup.py install --prefix=/usr --root=$(DESTDIR)

ubuntu_install: purge_pycache
	python3 setup.py install --prefix=/usr --root=$(DESTDIR) --no-compile

uninstall:
	$(eval DAEMONPATH := $(shell find $(DESTDIR)/usr/lib/python3* -maxdepth 2 -name "razer"))
	$(eval EGGPATH := $(shell readlink -f $(DAEMONPATH)-*.egg-info/)) # shows an error when it's not already installed (maybe use some if condition)
	@rm -v -rf $(DAEMONPATH)
	@rm -v -rf $(EGGPATH)

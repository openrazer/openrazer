DESTDIR?=/
PYTHONDIR?=`python3 -c 'import site; print(site.getsitepackages()[0])'`

purge_pycache:
	@find -name '__pycache__' | xargs rm -rf

install: purge_pycache
	@install -v -d razer $(DESTDIR)/$(PYTHONDIR)/razer
	@cp -v -r razer/* $(DESTDIR)/$(PYTHONDIR)/razer

uninstall:
	@rm -v -rf $(DESTDIR)/$(PYTHONDIR)/razer

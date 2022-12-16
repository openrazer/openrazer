# SPDX-License-Identifier: GPL-2.0-or-later

DESTDIR?=/

purge_pycache:
	@find -name '__pycache__' | xargs rm -rf

install: purge_pycache
	python3 setup.py install --prefix=/usr --root=$(DESTDIR)

ubuntu_install: purge_pycache
	python3 setup.py install --prefix=/usr --root=$(DESTDIR) --no-compile --install-layout=deb

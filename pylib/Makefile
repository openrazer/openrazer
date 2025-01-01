# SPDX-License-Identifier: GPL-2.0-or-later

DESTDIR?=/
PREFIX?=/usr

purge_pycache:
	find -name '__pycache__' | xargs rm -rf

build:
	python3 -m build --wheel --no-isolation --outdir dist/

install: purge_pycache build
	python3 -m installer --destdir=$(DESTDIR) --prefix=$(PREFIX) dist/*.whl

ubuntu_install: purge_pycache build
	DEB_PYTHON_INSTALL_LAYOUT=deb_system python3 -m installer --destdir=$(DESTDIR) --prefix=$(PREFIX) --no-compile-bytecode dist/*.whl

clean: purge_pycache
	rm -rf build/ dist/ *.egg-info

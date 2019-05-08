# ./convert.py <../debian/changelog >changelog
# rpmbuild -bb --build-in-place openrazer.spec

# gitcommit - git-commit source's tag for packages

%define dkms_name openrazer-driver
%define dkms_version 2.5.0

#define gitcommit 6ae1f7d55bf10cc6b5cb62a5ce99ff22c43e0701

Name: openrazer-meta
Version: 2.5.0
Release: 1.2%{?gitcommit:.%(tag="%{gitcommit}"; echo "${tag:0:7}")}
Summary: Open source software for managing Razer devices
License: GPL-2.0
URL: https://openrazer.github.io/
%if 0%{?_build_in_place:0}
%if 0%{?gitcommit:1}
Source0: https://github.com/openrazer/openrazer/archive/%{gitcommit}.tar.gz
%else
Source0: https://github.com/openrazer/openrazer/releases/download/v%{version}/openrazer-%{version}.tar.xz
%endif
%endif
BuildArch: noarch
Requires: openrazer-kernel-modules-dkms
Requires: openrazer-daemon
Requires: python3-openrazer

%description
Meta package for installing the OpenRazer driver, userspace daemon, a Python library and documentation.

Supported devices include keyboards, mice, mouse-mats, headsets and various other devices.


%package -n openrazer-kernel-modules-dkms
Summary: OpenRazer Driver DKMS package
Group: System Environment/Kernel
Obsoletes: razer-kernel-modules-dkms
Obsoletes: razer-chroma-driver
Provides: razer-kernel-modules-dkms
Provides: razer-chroma-driver
Conflicts: razer-kernel-modules-dkms
Conflicts: razer-chroma-driver
Requires: dkms
Requires: kernel-devel kernel-headers
Requires: udev
# OBS fails without that
%if 0%{?suse_version}
Requires(pre): shadow
Requires(post): dkms
%else
%if 0%{!?fedora:1}
Requires(pre): shadow-utils
%endif
Requires(post): dkms
%endif
%description -n openrazer-kernel-modules-dkms
Kernel driver for Razer devices, that will be build by DKMS from the source code.

Supported devices include keyboards, mice, mouse-mats, headsets and various other devices.

Kernel sources or headers are required to compile this module.

Please read the Troubleshooting Guide in /usr/share/doc/openrazer-driver-dkms/README.


%package -n openrazer-daemon
Summary: OpenRazer userspace daemon
Group: System Environment/Daemons
Obsoletes: razer-daemon
Provides: razer-daemon
Conflicts: razer-daemon
BuildRequires: python3-devel
BuildRequires: python3-setuptools
Requires: openrazer-kernel-modules-dkms
Requires: python3
# Thanks openSUSE for this great package name...
%if 0%{?suse_version}
Requires: dbus-1-python3
Requires: typelib(Gdk)
%else
Requires: python3-dbus
%endif
%if 0%{?mageia}
Requires: python3-gobject3
%else
Requires: python3-gobject
%endif
Requires: python3-setproctitle
Requires: python3-pyudev
Requires: python3-daemonize
Requires: xautomation
Requires: xdotool
%description -n openrazer-daemon
Userspace daemon that abstracts access to the kernel driver.

Provides a DBus service to interface with the driver.


%package -n python3-openrazer
Summary: OpenRazer Python library
Group: System Environment/Libraries
Obsoletes: python3-razer
Provides: python3-razer
Conflicts: python3-razer
BuildRequires: python3-devel
BuildRequires: python3-setuptools
Requires: openrazer-daemon
Requires: python3
# Thanks openSUSE for this great package name...
%if 0%{?suse_version}
Requires: dbus-1-python3
%else
Requires: python3-dbus
%endif
%if 0%{?mageia}
Requires: python3-gobject3
%else
Requires: python3-gobject
%endif
Requires: python3-numpy
%description -n python3-openrazer
Python library for interacting with the OpenRazer daemon.


%prep
#!/bin/sh
%if 0%{?_build_in_place:1}
pushd ..
%else
%if 0%{?gitcommit:1}
%autosetup -n openrazer-%{gitcommit}
pushd "openrazer-%{gitcommit}"
%else
%autosetup -n openrazer-%{version}
pushd "openrazer-%{version}"
%endif
%endif

%if 0%{fedora}
#FIX: replace 'plugdev'-group with 'input'-group for Fedora
for F in daemon driver examples install_files logo pylib scripts; do
find "$F" -type f | xargs sed -re 's/plugdev/input/' -i
done
%endif

popd


%build
%if 0%{?_build_in_place:1}
cd ..
%endif


%install
#!/bin/sh
%if 0%{?_build_in_place:1}
cd ..
%endif
rm -rf $RPM_BUILD_ROOT
# setup_dkms & udev_install -> razer-kernel-modules-dkms
# daemon_install -> razer_daemon
# python_library_install -> python3-razer
make DESTDIR=$RPM_BUILD_ROOT setup_dkms udev_install daemon_install python_library_install

install -Dm644 README.md "$RPM_BUILD_ROOT/%{_docdir}/openrazer/README.md"

%clean
rm -rf $RPM_BUILD_ROOT


%pre -n openrazer-kernel-modules-dkms
# noop

%post -n openrazer-kernel-modules-dkms
#!/bin/sh
set -e

# Only on initial installation
if [ "$1" == 1 ]; then
  dkms install %{dkms_name}/%{dkms_version}
fi

udevadm control -R

echo -e "\e[31m********************************************"
echo -e "\e[31m* To complete installation, please run:    *"
%if 0%{fedora}
echo -e "\e[31m* # su -c 'usermod -aGinput <yourUsername>'*"
%else
echo -e "\e[31m* # sudo gpasswd -a <yourUsername> input *"
%endif
echo -e "\e[31m********************************************"
echo -e -n "\e[39m"


%preun -n openrazer-kernel-modules-dkms
#!/bin/sh

# Only on uninstallation
if [ "$1" == 0 ]; then
  if [ "$(dkms status -m %{dkms_name} -v %{dkms_version})" ]; then
    dkms remove -m %{dkms_name} -v %{dkms_version} --all
  fi
fi


%files
# meta package is empty

%files -n openrazer-kernel-modules-dkms
%defattr(-,root,root,-)
# A bit hacky but it works
%{_udevrulesdir}/../razer_mount
%{_udevrulesdir}/99-razer.rules
%{_usrsrc}/%{dkms_name}-%{dkms_version}/

%files -n openrazer-daemon
%{_bindir}/openrazer-daemon
%{python3_sitelib}/openrazer_daemon/
%{python3_sitelib}/openrazer_daemon-*.egg-info/
%{_datadir}/openrazer/
%{_datadir}/dbus-1/services/org.razer.service
%{_prefix}/lib/systemd/user/openrazer-daemon.service
%{_mandir}/man5/razer.conf.5*
%{_mandir}/man8/openrazer-daemon.8*
%{_docdir}/openrazer/README.md

%files -n python3-openrazer
%{python3_sitelib}/openrazer/
%{python3_sitelib}/openrazer-*.egg-info/

%changelog
%include changelog

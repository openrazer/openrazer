# This spec file should work on Fedora, openSUSE and Mageia

%define dkms_name openrazer-driver
%define dkms_version 3.10.1

#define gitcommit 6ae1f7d55bf10cc6b5cb62a5ce99ff22c43e0701

Name: openrazer-meta
Version: 3.10.1
Release: 1%{?dist}
Summary: Open source driver and user-space daemon for managing Razer devices

License: GPL-2.0
URL: https://github.com/openrazer/openrazer

%if 0%{?gitcommit:1}
Source0: https://github.com/openrazer/openrazer/archive/%{gitcommit}.tar.gz
%else
Source0: https://github.com/openrazer/openrazer/releases/download/v%{version}/openrazer-%{version}.tar.xz
%endif

BuildArch: noarch

Requires: openrazer-kernel-modules-dkms
Requires: openrazer-daemon
Requires: python3-openrazer

%description
Meta package for installing all required openrazer packages.


%package -n openrazer-kernel-modules-dkms
Summary: OpenRazer Driver DKMS package
Group: System Environment/Kernel
Obsoletes: razer-kernel-modules-dkms
Provides: razer-kernel-modules-dkms
Requires: dkms
Requires: make
Requires: udev
# OBS fails without that
%if 0%{?suse_version}
Requires(pre): shadow
Requires(post): dkms
%else
Requires(pre): shadow-utils
%endif

%description -n openrazer-kernel-modules-dkms
Kernel driver for Razer devices (DKMS-variant)


%package -n openrazer-daemon
Summary: OpenRazer Service package
Group: System Environment/Daemons
Obsoletes: razer-daemon
Provides: razer-daemon
BuildRequires: python3-devel
BuildRequires: python3-setuptools
Requires: openrazer-kernel-modules-dkms
Requires: python3
%if 0%{?suse_version}
Requires: dbus-1-python3
Requires: typelib(Gdk) = 3.0
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

%description -n openrazer-daemon
Userspace daemon that abstracts access to the kernel driver. Provides a DBus service for applications to use.


%package -n python3-openrazer
Summary: OpenRazer Python library
Group: System Environment/Libraries
Obsoletes: python3-razer
Provides: python3-razer
BuildRequires: python3-devel
BuildRequires: python3-setuptools
Requires: openrazer-daemon
Requires: python3
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
Python library for accessing the daemon from Python.


%prep
%if 0%{?gitcommit:1}
%autosetup -n openrazer-%{gitcommit}
%else
%autosetup -n openrazer-%{version}
%endif

%build
# noop


%install
rm -rf $RPM_BUILD_ROOT
# setup_dkms & udev_install -> razer-kernel-modules-dkms
# daemon_install -> razer_daemon
# python_library_install -> python3-razer
make DESTDIR=$RPM_BUILD_ROOT setup_dkms udev_install daemon_install python_library_install


%clean
rm -rf $RPM_BUILD_ROOT


%pre -n openrazer-kernel-modules-dkms
#!/bin/sh
set -e

getent group plugdev >/dev/null || groupadd -r plugdev


%if 0%{?mageia}

%posttrans -n openrazer-kernel-modules-dkms
dkms add -m %{dkms_name} -v %{dkms_version} --rpm_safe_upgrade
dkms build -m %{dkms_name} -v %{dkms_version} --rpm_safe_upgrade
dkms install -m %{dkms_name} -v %{dkms_version} --rpm_safe_upgrade

echo -e "\e[31m********************************************"
echo -e "\e[31m* To complete installation, please run:    *"
echo -e "\e[31m* # sudo gpasswd -a <yourUsername> plugdev *"
echo -e "\e[31m********************************************"
echo -e -n "\e[39m"

%preun -n openrazer-kernel-modules-dkms
dkms remove -m %{dkms_name} -v %{dkms_version} --rpm_safe_upgrade --all

%else

%posttrans -n openrazer-kernel-modules-dkms
#!/bin/sh
set -e

dkms install %{dkms_name}/%{dkms_version}

echo -e "\e[31m********************************************"
echo -e "\e[31m* To complete installation, please run:    *"
echo -e "\e[31m* # sudo gpasswd -a <yourUsername> plugdev *"
echo -e "\e[31m********************************************"
echo -e -n "\e[39m"

%preun -n openrazer-kernel-modules-dkms
#!/bin/sh

if [ "$(dkms status -m %{dkms_name} -v %{dkms_version})" ]; then
  dkms remove -m %{dkms_name} -v %{dkms_version} --all
fi

%endif


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

%files -n python3-openrazer
%{python3_sitelib}/openrazer/
%{python3_sitelib}/openrazer-*.egg-info/

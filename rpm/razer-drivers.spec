# This spec file was tested on Fedora 25 and on OpenSUSE Leap 42.2.

%define dkms_name razer_chroma_driver
%define dkms_version 1.0.0

Name: razer-drivers
Version: 1.1.8
Release: 1%{?dist}
Summary: Razer drivers for Linux

License: GPL-2.0
URL: https://github.com/terrycain/razer-drivers

Source0: https://github.com/terrycain/razer-drivers/archive/v%{version}.tar.gz
BuildArch: noarch

Requires: razer-kernel-modules-dkms
Requires: razer-daemon
Requires: python3-razer

%description
Razer Driver DKMS package


%package -n razer-kernel-modules-dkms
Summary: Razer Driver DKMS package
Group: System Environment/Kernel
Requires: dkms
Requires: udev
# OBS fails without that
%if 0%{?suse_version}
Requires(pre): shadow
Requires(post): dkms
%else
Requires(pre): shadow-utils
%endif
%description -n razer-kernel-modules-dkms
Long description


%package -n razer-daemon
Summary: Razer Service package
Group: System Environment/Daemons
BuildRequires: python3-devel
BuildRequires: python3-setuptools
Requires: razer-kernel-modules-dkms
Requires: python3
# Thanks OpenSUSE for this great package name...
%if 0%{?suse_version}
Requires: dbus-1-python3
%else
Requires: python3-dbus
%endif
Requires: python3-gobject
Requires: python3-setproctitle
Requires: python3-pyudev
Requires: xautomation
Requires: xdotool
%description -n razer-daemon
Long description


%package -n python3-razer
Summary: Razer Python library
Group: System Environment/Libraries
BuildRequires: python3-devel
BuildRequires: python3-setuptools
Requires: razer-daemon
Requires: python3
# Thanks openeuse for this great package name...
%if 0%{?suse_version}
Requires: dbus-1-python3
%else
Requires: python3-dbus
%endif
Requires: python3-gobject
Requires: python3-numpy
%description -n python3-razer
Long description


%prep
%autosetup -n razer-drivers-%{version}

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


%pre -n razer-kernel-modules-dkms
#!/bin/sh
set -e

getent group plugdev >/dev/null || groupadd -r plugdev


%post -n razer-kernel-modules-dkms
#!/bin/sh
set -e

DKMS_NAME=%{dkms_name}
DKMS_VERSION=%{dkms_version}

# Only on initial installation
if [ "$1" == 1 ]; then
  dkms install $DKMS_NAME/$DKMS_VERSION
fi


%preun -n razer-kernel-modules-dkms
#!/bin/sh

DKMS_NAME=%{dkms_name}
DKMS_VERSION=%{dkms_version}

# Only on uninstallation
if [ "$1" == 0 ]; then
  if [ "$(dkms status -m $DKMS_NAME -v $DKMS_VERSION)" ]; then
    dkms remove -m $DKMS_NAME -v $DKMS_VERSION --all
  fi
fi

%files
# meta package is empty


%files -n razer-kernel-modules-dkms
%defattr(-,root,root,-)
# A bit hacky but it works
%{_udevrulesdir}/../razer_mount
%{_udevrulesdir}/99-razer.rules
%{_usrsrc}/%{dkms_name}-%{dkms_version}/

%files -n razer-daemon
%{_sysconfdir}/xdg/autostart/razer-service.desktop
%{_bindir}/razer-service
%{python3_sitelib}/razer_daemon/
%{python3_sitelib}/razer_daemon-*.egg-info/
%{_datadir}/razer-service/
%{_mandir}/man5/razer.conf.5.gz
%{_mandir}/man8/razer-service.8.gz

%files -n python3-razer
%{python3_sitelib}/razer/
%{python3_sitelib}/razer-*.egg-info/

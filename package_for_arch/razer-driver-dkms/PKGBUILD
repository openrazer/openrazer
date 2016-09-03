# Maintainer: Gabriele Musco <emaildigabry@gmail.com>
# Upstream URL: https://github.com/terrycain/razer_drivers

_pkgbase=razer-driver
pkgname=razer-driver-dkms
pkgver=1.0.5
pkgrel=2
pkgdesc="An entirely open source driver for managing Razer peripherals on Linux. (DKMS)"
arch=('x86_64')
url="https://github.com/terrycain/razer_drivers"
license=('GPLv3')
depends=('dkms' 'udev')
makedepends=('git' 'make')
conflicts=("${_pkgbase}")
install=${pkgname}.install
source=("${_pkgbase}::git+git://github.com/terrycain/razer_drivers.git#tag=v$pkgver")
md5sums=('SKIP')

package() {
  #part1: dkms driver
  mkdir -p $pkgdir/usr/share
  mkdir -p $pkgdir/usr/src/$_pkgbase-$pkgver
  mkdir -p $pkgdir/usr/lib/udev/rules.d

  cp -r $srcdir/$_pkgbase/driver $pkgdir/usr/src/$_pkgbase-$pkgver
  cp $srcdir/$_pkgbase/Makefile $pkgdir/usr/src/$_pkgbase-$pkgver
  cp $srcdir/$_pkgbase/install_files/dkms/dkms.conf $pkgdir/usr/src/$_pkgbase-$pkgver
  
  #part2: udev rules
  cp $srcdir/$_pkgbase/install_files/udev/99-razer.rules $pkgdir/usr/lib/udev/rules.d
  cp $srcdir/$_pkgbase/install_files/udev/razer_mount $pkgdir/usr/lib/udev
}

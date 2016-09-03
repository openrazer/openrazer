# Maintainer: Gabriele Musco <emaildigabry@gmail.com>
# Upstream URL: https://github.com/terrycain/razer_drivers

pkgname=razer-daemon
pkgver=1.0.5
pkgrel=2
pkgdesc="A daemon for controlling razer-driver"
arch=('any')
url="https://github.com/terrycain/razer_drivers"
license=('GPLv3')
depends=('razer-driver-dkms' 'python' 'python-dbus' 'python-gobject' 'python-setproctitle' 'xautomation' 'xdotool' 'libdbus' 'python-notify2')
makedepends=('git')
optdepends=('python-razer: python library for controlling the daemon')
install=${pkgname}.install
source=("${pkgname}::git+git://github.com/terrycain/razer_drivers.git#tag=v$pkgver")
md5sums=('SKIP')

package() {
  # use the make file instead of copying manually? maybe, not now.
  mkdir -p $pkgdir/usr/share/razer-service
  mkdir -p $pkgdir/usr/share/man/man5
  mkdir -p $pkgdir/usr/share/man/man8
  mkdir -p $pkgdir/usr/lib/python3.5/site-packages
  mkdir -p $pkgdir/usr/bin
  mkdir -p $pkgdir/etc/xdg/autostart

  cp -r $srcdir/$pkgname/daemon/razer_daemon $pkgdir/usr/lib/python3.5/site-packages/
  cp $srcdir/$pkgname/daemon/run_razer_daemon.py $pkgdir/usr/bin/razer-service
  cp $srcdir/$pkgname/daemon/resources/razer.conf $pkgdir/usr/share/razer-service/razer.conf.example

  cp -v $srcdir/$pkgname/install_files/desktop/razer-service.desktop $pkgdir/etc/xdg/autostart/razer-service.desktop
  
  gzip -c $srcdir/$pkgname/daemon/resources/man/razer.conf.5 > $pkgdir/usr/share/man/man5/razer.conf.5.gz
  gzip -c $srcdir/$pkgname/daemon/resources/man/razer-service.8 > $pkgdir/usr/share/man/man8/razer-service.8.gz
}

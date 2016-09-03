# Maintainer: Gabriele Musco <emaildigabry@gmail.com>
# Upstream URL: https://github.com/terrycain/razer_drivers

pkgname=python-razer
pkgver=1.0.5
pkgrel=2
pkgdesc="A python library for controlling razer-daemon"
arch=('any')
url="https://github.com/terrycain/razer_drivers"
license=('GPLv3')
depends=('razer-daemon' 'python' 'python-dbus' 'python-numpy')
makedepends=('git')
optdepends=('razercommander-git: gtk frontend for razer-drivers')
install=${pkgname}.install
source=("${pkgname}::git+git://github.com/terrycain/razer_drivers.git#tag=v$pkgver")
md5sums=('SKIP')

package() {
  mkdir -p $pkgdir/usr/lib/python3.5/site-packages

  cp -r $srcdir/$pkgname/pylib/razer $pkgdir/usr/lib/python3.5/site-packages/
}

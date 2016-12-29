# RPM

The spec file was tested on Fedora 25 and openSUSE Leap 42.2 and openSUSE Tumbleweed.

Binary packages are available at https://software.opensuse.org/download.html?project=home%3Az3ntu&package=razer-drivers

### How to build with rpmbuild
```
spectool -g razer-drivers.spec # download the source
rpmbuild -bb razer-drivers.spec # build the package
```

### How to build with osc
```
osc checkout home:z3ntu razer-drivers
cd home\:z3ntu/razer-drivers/
osc build
```

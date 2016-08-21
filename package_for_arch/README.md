# Installing `razer_chroma_driver` on Arch Linux

*This tutorial is pretty basic. If you're an advanced user, just skip it. You have to compile the packages using the provided PKGBUILDs, and install them in order: dkms, daemon, python*

## 0. Getting started

- Clone this repo:

		$ git clone git://github.com/terrycain/razer_drivers.git

- Get into the **package_for_arch** folder:

		$ cd razer_drivers/package_for_arch

## 1. Installing the driver

- Get into the **razer-driver-dkms** folder:

		$ cd razer-driver-dkms

- Create the Arch package:

		$ makepkg -s

- Install the driver package you just compiled:

		$ sudo pacman -U *.pkg.tar

- Get back to the previous folder:

		$ cd ..

## 2. Installing the daemon

- Get into the **razer-driver-daemon** folder:

		$ cd razer-driver-daemon

- Create the Arch package:

		$ makepkg -s

- Install the daemon package you just compiled:

		$ sudo pacman -U *.pkg.tar

- Get back to the previous folder:

		$ cd ..

##3. Installing the python library

- Get into the **python-razer** folder:

		$ cd python-razer

- Create the Arch package:

		$ makepkg -s

- Install the python library package you just compiled:

		$ sudo pacman -U *.pkg.tar

##4. Finishing up

- Reboot your PC.

You may need to manually bind your Razer device to the driver, if that's your case, please follow this simple tutorial:

### [Setting up the keyboard driver](https://github.com/pez2001/razer_chroma_drivers/wiki/Setting-up-the-keyboard-driver)

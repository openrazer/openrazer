#Installing `razer_chroma_driver` on Arch Linux

**Notice #1: don't run these commands directly as root. Run them as a normal user, with sudo privileges, and just put your password in whenever it's prompted**

**Notice #2: this PKGBUILD is kinda incomplete. It only installs the driver (the dkms kernel module) but it leaves behind the daemon and python library. This is a temporary issue and will be solved in the close future.**

- Clone this repo:

		git clone git://github.com/terrycain/razer_drivers.git

- Get into the *package_for_arch* folder:

		cd razer_drivers/package_for_arch

- Create the Arch package:

		makepkg -s

- Install the newly made package:

		sudo pacman -U *.pkg.tar

- Reboot your PC.

You may need to manually bind your Razer device to the driver, if that's your case, please follow this simple tutorial:

###[Setting up the keyboard driver](https://github.com/pez2001/razer_chroma_drivers/wiki/Setting-up-the-keyboard-driver)
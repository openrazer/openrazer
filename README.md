# Razer Drivers

A collection of Linux drivers for the Razer devices, providing kernel drivers, DBus services and python bindings to interact with the DBus interface.

Website: [Here](https://terrycain.github.io/razer-drivers/)

**Before raising an issue** saying something doesn't work, read [this](https://github.com/terrycain/razer-drivers/wiki/Troubleshooting) Wiki page, try not to create new issues if one exists, reopen it.

## Driver Support
### Keyboard Support:
 * Razer Blackwidow Classic *(full support)*
 * Razer Blackwidow Ultimate 2012 *(full support)*
 * Razer Blackwidow Ultimate 2013 *(full support)*
 * Razer Blackwidow Chroma *(full support)*
 * Razer Blackwidow Chroma Tournament Edition *(full support)*
 * Razer Blackwidow X Chroma *(probably full support)*
 * Razer Blade Stealth *(full support)*
 * Razer Blackwidow Ultimate 2016 *(full support)*

### Mousemat Support:
 * Razer Firefly *(full support)*

### Mouse Support:
 * Razer Mamba (Wired) *(work in progress)*
 * Razer Mamba (Wireless) *(full support)*
 * Razer Mamba TE *(work in progress)*
 * Razer Abyssus *(full support)*

### Misc Peripheral Support
 * Razer Tartarus *(full support)*

---

## Daemon Support
I've created a daemon that lives in userspace which allows one to easily interact with the driver. It abstracts away some of the logic and also (will eventually) allow you to build
custom effects, though it does do mulitcoloured ripples :). As every device is slightly different I've maintained a mapping between devices supported by the driver and devices supported
by the daemon. If your device is supported by the driver but not by the daemon then raise an issue.

### Keyboard Support:
 * Razer Blackwidow Ultimate 2013
 * Razer Blackwidow Chroma
 * Razer Blackwidow Chroma Tournament Edition
 * Razer Blackwidow X Chroma
 * Razer Blade Stealth

### Mousemat Support:
 * Razer Firefly

### Mouse Support:
 * Razer Mamba (Wireless)

## Installation

### Arch Linux

[Arch Linux installation guide](https://github.com/terrycain/razer_drivers/blob/master/package_for_arch/README.md) 

### Ubuntu Linux
We have a PPA here - `https://launchpad.net/~terrz/+archive/ubuntu/razerutils`


### Other distributions

[Here](https://github.com/terrycain/razer-drivers/wiki/Installation) is a page documenting the installation and uninstallation procedures.

## Applications

The following applications compliment and interact with this driver:

* [Polychromatic Controller](https://github.com/lah7/polychromatic-controller) - a graphical management tool and tray applet to managing Razer peripherals.
* [MacroW](https://github.com/igorbb/MacroW) - a simple tool to record and play keyboard macros. (though the dameon does on the fly recording ;) )
* [RazerCommander](https://github.com/GabMus/razerCommander) - Simple GUI written in Gtk3

## Status of Code

 - **Driver:** Release Candidate
 - **Daemon:** Beta
 - **Client Library:** Beta
 - **Packages:** Beta

##Contributions

Some guys have already donated to the cause. ;-) Feel free.
The parent of the fork had a list of what would be done if money was donated. I don't need the money (though you can never have enough right ;-) ), but obviously its a great motivator.
If you want you device and your not willing to get one to me then if your willing to follow [this](https://github.com/terrycain/razer-drivers/wiki/Reverse-Engineering-USB-Protocol) then support can be added.

---

The project is licensed under the GPL and is not affiliated with [Razer, Inc](http://www.razerzone.com/).

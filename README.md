[![Codacy Badge](https://api.codacy.com/project/badge/Grade/fd36d0d76e9842c4a7d67118bd01b275)](https://www.codacy.com/app/terry_5/razer-drivers?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=terrycain/razer-drivers&amp;utm_campaign=Badge_Grade)

# Razer Drivers

A collection of Linux drivers for the Razer devices, providing kernel drivers, DBus services and python bindings to interact with the DBus interface.

Website: [Here](https://terrycain.github.io/razer-drivers/)

**Before raising an issue** saying something doesn't work, read [this](https://github.com/terrycain/razer-drivers/wiki/Troubleshooting) Wiki page, try not to create new issues if one exists, reopen it.

## Device Support
### Keyboards
| Device                                      | Driver | Daemon |
| ------------------------------------------- | ------ | ------ |
| Razer BlackWidow Classic                    |   ✔    |        |
| Razer BlackWidow Ultimate 2012              |   ✔    |        |
| Razer BlackWidow Ultimate 2013              |   ✔    |   ✔    |
| Razer BlackWidow Ultimate 2016              |   ✔    |   ✔    |
| Razer BlackWidow Chroma                     |   ✔    |   ✔    |
| Razer BlackWidow Chroma Tournament Edition  |   ✔    |   ✔    |
| Razer BlackWidow X Chroma                   |   ✔    |   ✔    |
| Razer Blade Stealth                         |   ✔    |   ✔    |

### Mouse
| Device                          | Driver | Daemon |
| ------------------------------- | ------ | ------ |
| Razer Mamba (Wireless)          |   ✔    |   ✔    |
| Razer Mamba (Wired)             |   ✔    |   ✔    |
| Razer Mamba Tournament Edition  |   ✔    |   ✔    |
| Razer Abyssus 2014              |   ✔    |   ✔    |

### Mousemats
| Device        | Driver | Daemon |
| ------------- | ------ | ------ |
| Razer Firefly |   ✔    |   ✔    |

### Peripherals
| Device          | Driver | Daemon |
| --------------- | ------ | ------ |
| Razer Tartarus  |   ✔    |   ✔    |

### Daemon Support
I've created a daemon that lives in userspace which allows one to easily interact with the driver. It abstracts away some of the logic and also (will eventually) allow you to build
custom effects, though it does do mulitcoloured ripples :). As every device is slightly different I've maintained a mapping between devices supported by the driver and devices supported
by the daemon. If your device is supported by the driver but not by the daemon then [raise an issue](https://github.com/terrycain/razer-drivers/issues/new).

---

## Installation

### Arch Linux

Install `razer-driver-meta` from AUR, or install the three packages `razer-driver-dkms`, `razer-daemon` and `python-razer` singularly from AUR.

### Ubuntu Linux
We have a PPA here - `https://launchpad.net/~terrz/+archive/ubuntu/razerutils`

```
sudo add-apt-repository ppa:terrz/razerutils
sudo apt update
sudo apt install python3-razer razer-kernel-modules-dkms razer-daemon razer-doc
```

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

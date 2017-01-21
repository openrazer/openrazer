[![Codacy Badge](https://api.codacy.com/project/badge/Grade/fd36d0d76e9842c4a7d67118bd01b275)](https://www.codacy.com/app/terry_5/razer-drivers?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=terrycain/razer-drivers&amp;utm_campaign=Badge_Grade)

---------------------

# Razer Drivers

A collection of Linux drivers for the Razer devices, providing kernel drivers, DBus services and python bindings to interact with the DBus interface.

[See the website](https://terrycain.github.io/razer-drivers/)

**Before raising an issue** saying something doesn't work, read [this](https://github.com/terrycain/razer-drivers/wiki/Troubleshooting) Wiki page, try not to create new issues if one exists, reopen it.

Also this is the master branch, devices may appear in the tables below but might not be released in package form yet. Check out the stable branch for what _should_ be in the packages.

## Device Support
### Keyboards
| Device                                        | USB VID:PID | Driver | Daemon | Misc |
| --------------------------------------------- | ----------- | ------ | ------ | ---- |
| Razer BlackWidow Ultimate 2012                |  1532:010D  |   ✔    |   ✔    |      |
| Razer Anansi                                  |  1532:010F  |   ✔    |   ✔    |      |
| Razer BlackWidow Ultimate 2013                |  1532:011A  |   ✔    |   ✔    |      |
| Razer BlackWidow Classic                      |  1532:011B  |   ✔    |   ✔    |      |
| Razer BlackWidow Chroma                       |  1532:0203  |   ✔    |   ✔    |      |
| Razer DeathStalker Chroma                     |  1532:0204  |   ✔    |   ✔    |      |
| Razer Blade Stealth                           |  1532:0205  |   ✔    |   ✔    |      |
| Razer BlackWidow Tournament Edition Chroma    |  1532:0209  |   ✔    |   ✔    |      |
| Razer Blade QHD                               |  1532:020F  |   ✔    |   ✔    | Individual Key lighting not working yet |
| Razer Blade Pro (Late 2016)                   |  1532:0210  |   ✔    |   ✔    |      |
| Razer BlackWidow Ultimate 2016                |  1532:0214  |   ✔    |   ✔    |      |
| Razer BlackWidow X Chroma                     |  1532:0216  |   ✔    |   ✔    |      |
| Razer BlackWidow X Ultimate                   |  1532:0217  |   ✔    |   ✔    |      |
| Razer BlackWidow X Tournament Edition Chroma  |  1532:021A  |   ✔    |   ✔    |      |
| Razer Ornata Chroma                           |  1532:021E  |   ✔    |   ✔    |      |
| Razer Ornata                                  |  1532:021F  |   ✔    |   ✔    |      |
| Razer Blade Stealth (Late 2016)               |  1532:0220  |   ✔    |   ✔    |      |

### Mouse
| Device                          | USB VID:PID | Driver | Daemon |
| ------------------------------- | ----------- | ------ | ------ |
| Razer Imperator 2012            |  1532:002F  |   ✔    |   ✔    |
| Razer Ouroboros 2012            |  1532:0032  |   ✔    |   ✔    |
| Razer Abyssus 2014              |  1532:0042  |   ✔    |   ✔    |
| Razer DeathAdder Chroma         |  1532:0043  |   ✔    |   ✔    |
| Razer Mamba (Wired)             |  1532:0044  |   ✔    |   ✔    |
| Razer Mamba (Wireless)          |  1532:0045  |   ✔    |   ✔    |
| Razer Mamba Tournament Edition  |  1532:0046  |   ✔    |   ✔    |
| Razer Orochi (Wired)            |  1532:0048  |   ✔    |   ✔    |
| Razer Diamondback Chroma        |  1532:004C  |   ✔    |   ✔    |
| Razer Naga Hex V2               |  1532:0050  |   ✔    |   ✔    |
| Razer DeathAdder Elite          |  1532:005c  |   ✔    |   ✔    |

### Mousemats
| Device        | USB VID:PID | Driver | Daemon |
| ------------- | ----------- | ------ | ------ |
| Razer Firefly |  1532:0C00  |   ✔    |   ✔    |

### Headsets
| Device                   | USB VID:PID | Driver | Daemon |
| ------------------------ | ----------- | ------ | ------ |
| Razer Kraken 7.1 Chroma  |  1532:0504  |   ✔    |   ✔    |
| Razer Kraken 7.1 V2      |  1532:0510  |   ✔    |   ✔    |

### Misc
| Device                  | USB VID:PID | Driver | Daemon |
| ----------------------- | ----------- | ------ | ------ |
| Razer Chroma Mug Holder |  1532:0F07  |   ✔    |   ✔    |
| Razer Tartarus Chroma   |  1532:0208  |   ✔    |   ✔    |
| Razer Orbweaver         |  1532:0113  |   ✔    |   ✔    |

#### Determining the Device ID
Razer's devices use a VID (Vendor ID) of `1532`. You can identify the USB PID (Product ID) by typing:

    lsusb | grep '1532:'

This will output something similar to this:

    Bus 003 Device 005: ID 1532:0203 Razer USA, Ltd


-------------------

### No device found
Those of you on Ubuntu 16.10, some of you on 16.04 and at least some arch users might notice that even though your device is listed as supported it doesnt appear on any of the supporting applications. Udev is not playing ball. Look at [this](https://github.com/terrycain/razer-drivers/issues/67#issuecomment-260419314) comment and run it. Am currently working on a little service that should bind devices and set up the permissions like udev would.

-------------------

### Daemon Support
I've created a daemon that lives in userspace which allows one to easily interact with the driver. It abstracts away some of the logic and also (will eventually) allow you to build
custom effects, though it does do mulitcoloured ripples :). As every device is slightly different I've maintained a mapping between devices supported by the driver and devices supported
by the daemon. If your device is supported by the driver but not by the daemon then [raise an issue](https://github.com/terrycain/razer-drivers/issues/new).

---

## Installation

Packages are available for these distributions:

* [Ubuntu / Linux Mint](https://terrycain.github.io/razer-drivers/#ubuntu)
* [Arch](https://terrycain.github.io/razer-drivers/#arch)
* [Fedora](https://terrycain.github.io/razer-drivers/#fedora)
* [OpenSUSE](https://terrycain.github.io/razer-drivers/#opensuse)

Other distributions can manually install [using these instructions.](https://terrycain.github.io/razer-drivers/#other)

## Applications

The following applications complement and interact with this driver:

* [Polychromatic](https://github.com/lah7/polychromatic) - a graphical management tool and tray applet to managing Razer peripherals.
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

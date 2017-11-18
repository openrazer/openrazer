# [OpenRazer](https://openrazer.github.io/)

A collection of Linux drivers for the Razer devices, providing kernel drivers, DBus services and python bindings to interact with the DBus interface.

**Before raising an issue** saying something doesn't work, read [the troubleshooting wiki page](https://github.com/openrazer/openrazer/wiki/Troubleshooting), try not to create new issues if one exists.

Also this is the master branch, devices may appear in the tables below but might not be released in package form yet. Check out the stable branch for what _should_ be in the packages.

## Device Support
### Keyboards
| Device                                        | USB VID:PID | Driver | Daemon | Notes |
| --------------------------------------------- | ----------- | ------ | ------ | ---- |
| Razer BlackWidow Ultimate 2012                |  1532:010D  |   ✔    |   ✔    |      |
| Razer BlackWidow Classic (Alternate)          |  1532:010E  |   ✔    |   ✔    |      |
| Razer Anansi                                  |  1532:010F  |   ✔    |   ✔    |      |
| Razer BlackWidow Ultimate 2013                |  1532:011A  |   ✔    |   ✔    |      |
| Razer BlackWidow Classic                      |  1532:011B  |   ✔    |   ✔    |      |
| Razer DeathStalker Expert                     |  1532:0202  |   ✔    |   ✔    |      |
| Razer BlackWidow Chroma                       |  1532:0203  |   ✔    |   ✔    |      |
| Razer DeathStalker Chroma                     |  1532:0204  |   ✔    |   ✔    |      |
| Razer Blade Stealth                           |  1532:0205  |   ✔    |   ✔    |      |
| Razer Orbweaver Chroma                        |  1532:0207  |   ✔    |   ✔    |      |
| Razer BlackWidow Tournament Edition Chroma    |  1532:0209  |   ✔    |   ✔    |      |
| Razer Blade QHD                               |  1532:020F  |   ✔    |   ✔    |      |
| Razer Blade Pro (Late 2016)                   |  1532:0210  |   ✔    |   ✔    |      |
| Razer BlackWidow Chroma (Overwatch)           |  1532:0211  |   ✔    |   ✔    |      |
| Razer BlackWidow Ultimate 2016                |  1532:0214  |   ✔    |   ✔    |      |
| Razer BlackWidow X Chroma                     |  1532:0216  |   ✔    |   ✔    |      |
| Razer BlackWidow X Ultimate                   |  1532:0217  |   ✔    |   ✔    |      |
| Razer BlackWidow X Tournament Edition Chroma  |  1532:021A  |   ✔    |   ✔    |      |
| Razer Ornata Chroma                           |  1532:021E  |   ✔    |   ✔    |      |
| Razer Ornata                                  |  1532:021F  |   ✔    |   ✔    |      |
| Razer Blade Stealth (Late 2016)               |  1532:0220  |   ✔    |   ✔    |      |
| Razer BlackWidow Chroma V2                    |  1532:0221  |   ✔    |   ✔    |      |
| Razer Blade (Late 2016)                       |  1532:0224  |   ✔    |   ✔    |      |
| Razer Blade Stealth (Mid 2017)                |  1532:022D  |   ✔    |   ✔    | Working except ripple |
| Razer Blade Pro (2017)                        |  1532:0225  |   ✔    |   ✔    | Ripple partially working |
| Razer Blade Pro FullHD (2017)                 |  1532:022F  |   ✔    |   ✔    | Working except ripple |
| Razer Blade Stealth (Late 2017)               |  1532:0232  |   ✔    |   ✔    | Working except ripple |

### Mouse
| Device                          | USB VID:PID | Driver | Daemon |
| ------------------------------- | ----------- | ------ | ------ |
| Razer Orochi 2011               |  1532:0013  |   ✔    |   ✔    |
| Razer Mamba 2012 (Wired)        |  1532:0024  |   ✔    |   ✔    |
| Razer Mamba 2012 (Wireless)     |  1532:0025  |   ✔    |   ✔    |
| Razer Imperator 2012            |  1532:002F  |   ✔    |   ✔    |
| Razer Ouroboros 2012            |  1532:0032  |   ✔    |   ✔    |
| Razer Taipan                    |  1532:0034  |   ✔    |   ✔    |
| Razer Naga Hex (Red)            |  1532:0036  |   ✔    |   ✔    |
| Razer Orochi 2013               |  1532:0039  |   ✔    |   ✔    |
| Razer Naga 2014                 |  1532:0040  |   ✔    |   ✔    |
| Razer Naga Hex                  |  1532:0041  |   ✔    |   ✔    |
| Razer Abyssus 2014              |  1532:0042  |   ✔    |   ✔    |
| Razer DeathAdder Chroma         |  1532:0043  |   ✔    |   ✔    |
| Razer Mamba (Wired)             |  1532:0044  |   ✔    |   ✔    |
| Razer Mamba (Wireless)          |  1532:0045  |   ✔    |   ✔    |
| Razer Mamba Tournament Edition  |  1532:0046  |   ✔    |   ✔    |
| Razer Orochi (Wired)            |  1532:0048  |   ✔    |   ✔    |
| Razer Diamondback Chroma        |  1532:004C  |   ✔    |   ✔    |
| Razer Naga Hex V2               |  1532:0050  |   ✔    |   ✔    |
| Razer Naga Chroma               |  1532:0053  |   ✔    |   ✔    |
| Razer Abyssus V2                |  1532:005B  |   ✔    |   ✔    |
| Razer DeathAdder Elite          |  1532:005C  |   ✔    |   ✔    |

### Mousemats
| Device        | USB VID:PID | Driver | Daemon |
| ------------- | ----------- | ------ | ------ |
| Razer Firefly |  1532:0C00  |   ✔    |   ✔    |

### Headsets
| Device                   | USB VID:PID | Driver | Daemon |
| ------------------------ | ----------- | ------ | ------ |
| Razer Kraken 7.1 Classic |  1532:0501  |   ✔    |   ✔    |
| Razer Kraken 7.1 Chroma  |  1532:0504  |   ✔    |   ✔    |
| Razer Kraken 7.1 V2      |  1532:0510  |   ✔    |   ✔    |

### Misc
| Device                  | USB VID:PID | Driver | Daemon |
| ----------------------- | ----------- | ------ | ------ |
| Razer Nostromo          |  1532:0111  |   ✔    |   ✔    |
| Razer Orbweaver         |  1532:0113  |   ✔    |   ✔    |
| Razer Tartarus          |  1532:0201  |   ✔    |   ✔    |
| Razer Tartarus Chroma   |  1532:0208  |   ✔    |   ✔    |
| Razer Core              |  1532:0215  |   ✔    |   ✔    |
| Razer Chroma Mug Holder |  1532:0F07  |   ✔    |   ✔    |

#### Determining the Device ID
Razer devices use a USB VID (Vendor ID) of `1532`. You can identify the USB PID (Product ID) by typing:

    lsusb | grep '1532:'

This will output something similar to this:

    Bus 003 Device 005: ID 1532:0203 Razer USA, Ltd


---

## Installation

Packages are available for these distributions:

* [Ubuntu / Linux Mint](https://openrazer.github.io/#ubuntu)
* [Debian](https://openrazer.github.io/#debian)
* [Arch Linux](https://openrazer.github.io/#arch)
* [Fedora](https://openrazer.github.io/#fedora)
* [openSUSE](https://openrazer.github.io/#opensuse)
* [Gentoo](https://openrazer.github.io/#gentoo)

## Applications

The following applications complement and interact with this driver:

* [Polychromatic](https://github.com/lah7/polychromatic) - a graphical management tool and tray applet to managing Razer peripherals.
* [RazerGenie](https://github.com/z3ntu/RazerGenie) - Qt application for configuring your Razer devices under GNU/Linux.
* [MacroW](https://github.com/igorbb/MacroW) - a simple tool to record and play keyboard macros. (though the dameon does on the fly recording ;) )
* [RazerCommander](https://github.com/GabMus/razerCommander) - Simple GUI written in Gtk3
* [Chroma Feedback](https://github.com/redaxmedia/chroma-feedback) - Turn your Razer keyboard, mouse or headphone into a extreme feedback device for Travis CI

## Status of Code

 - **Driver:** Release Candidate
 - **Daemon:** Beta
 - **Client Library:** Beta
 - **Packages:** Beta

## Contributions

You can donate to [@terrycain](https://github.com/terrycain) with PayPal to terry@terrys-home.co.uk .

---

The project is licensed under the GPL and is not officially endorsed by [Razer, Inc](http://www.razerzone.com/).

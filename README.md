# [OpenRazer](https://openrazer.github.io/)

A collection of Linux drivers for Razer devices - providing kernel drivers, DBus services and Python bindings to interact with the DBus interface.

## Something not working?

Sometimes there are problems with the driver installation due to missing kernel modules or secure boot. Please refer to the [Troubleshooting wiki page](https://github.com/openrazer/openrazer/wiki/Troubleshooting) for guidance.

If the troubleshooting guide did not pinpoint the problem, [try searching open/closed issues](https://github.com/openrazer/openrazer/issues?q=is%3Aissue+) before creating a new one.

## Device Support

The list below contains latest devices supported on this branch (usually **master**). These might not be released yet, so check the [stable branch](https://github.com/openrazer/openrazer/tree/stable) for what should be available in the packages for your distribution.

The devices below are fully feature supported by OpenRazer, which means all available USB controllable functions for that product are made available by the library.

### Keyboards
| Device                                        | USB VID:PID |
| --------------------------------------------- | ----------- |
| Razer BlackWidow Ultimate 2012                |  1532:010D  |
| Razer BlackWidow Classic (Alternate)          |  1532:010E  |
| Razer Anansi                                  |  1532:010F  |
| Razer BlackWidow Ultimate 2013                |  1532:011A  |
| Razer BlackWidow Stealth                      |  1532:011B  |
| Razer DeathStalker Expert                     |  1532:0202  |
| Razer BlackWidow Chroma                       |  1532:0203  |
| Razer DeathStalker Chroma                     |  1532:0204  |
| Razer Blade Stealth                           |  1532:0205  |
| Razer BlackWidow Tournament Edition Chroma    |  1532:0209  |
| Razer Blade QHD                               |  1532:020F  |
| Razer Blade Pro (Late 2016)                   |  1532:0210  |
| Razer BlackWidow Chroma (Overwatch)           |  1532:0211  |
| Razer BlackWidow Ultimate 2016                |  1532:0214  |
| Razer BlackWidow X Chroma                     |  1532:0216  |
| Razer BlackWidow X Ultimate                   |  1532:0217  |
| Razer BlackWidow X Tournament Edition Chroma  |  1532:021A  |
| Razer Ornata Chroma                           |  1532:021E  |
| Razer Ornata                                  |  1532:021F  |
| Razer Blade Stealth (Late 2016)               |  1532:0220  |
| Razer BlackWidow Chroma V2                    |  1532:0221  |
| Razer Blade (Late 2016)                       |  1532:0224  |
| Razer Blade Pro (2017)                        |  1532:0225  |
| Razer Huntsman Elite                          |  1532:0226  |
| Razer Huntsman                                |  1532:0227  |
| Razer BlackWidow Elite                        |  1532:0228  |
| Razer Cynosa Chroma                           |  1532:022A  |
| Razer Cynosa Chroma Pro                       |  1532:022C  |
| Razer Blade Stealth (Mid 2017)                |  1532:022D  |
| Razer Blade Pro FullHD (2017)                 |  1532:022F  |
| Razer Blade Stealth (Late 2017)               |  1532:0232  |
| Razer Blade 15 (2018)                         |  1532:0233  |
| Razer Blade Pro 17 (2019)                     |  1532:0234  |
| Razer BlackWidow Lite (2018)                  |  1532:0235  |
| Razer BlackWidow Essential                    |  1532:0237  |
| Razer Blade Stealth (2019)                    |  1532:0239  |
| Razer Blade 15 (2019) Advanced                |  1532:023A  |
| Razer Blade 15 (2018) Base Model              |  1532:023B  |
| Razer Cynosa Lite                             |  1532:023F  |
| Razer Blade 15 (2018) Mercury                 |  1532:0240  |
| Razer BlackWidow (2019)                       |  1532:0241  |
| Razer Huntsman Tournament Edition             |  1532:0243  |
| Razer Blade 15 (Mid 2019) Mercury             |  1532:0245  |
| Razer Blade 15 (Mid 2019) Base                |  1532:0246  |
| Razer Blade Stealth (Late 2019)               |  1532:024A  |
| Razer Blade Pro (Late 2019)                   |  1532:024C  |
| Razer Blade 15 Studio Edition (2019)          |  1532:024D  |
| Razer Blade Stealth (Early 2020)              |  1532:0252  |
| Razer Blade 15 Advanced (2020)                |  1532:0253  |
| Razer Blade 15 (Early 2020) Base              |  1532:0255  |
| Razer Huntsman Mini                           |  1532:0257  |
| Razer Blade Stealth (Late 2020)               |  1532:0259  |
| Razer Ornata Chroma V2                        |  1532:025D  |
| Razer Cynosa V2                               |  1532:025E  |
| Razer Book 13 (2020)                          |  1532:026A  |

### Mice
| Device                                        | USB VID:PID |
| --------------------------------------------- | ----------- |
| Razer Orochi 2011                             |  1532:0013  |
| Razer DeathAdder 3.5G                         |  1532:0016  |
| Razer Abyssus 1800                            |  1532:0020  |
| Razer Mamba 2012 (Wired)                      |  1532:0024  |
| Razer Mamba 2012 (Wireless)                   |  1532:0025  |
| Razer Naga 2012                               |  1532:002E  |
| Razer Imperator 2012                          |  1532:002F  |
| Razer Ouroboros 2012                          |  1532:0032  |
| Razer Taipan                                  |  1532:0034  |
| Razer Naga Hex (Red)                          |  1532:0036  |
| Razer DeathAdder 2013                         |  1532:0037  |
| Razer DeathAdder 1800                         |  1532:0038  |
| Razer Orochi 2013                             |  1532:0039  |
| Razer Naga 2014                               |  1532:0040  |
| Razer Naga Hex                                |  1532:0041  |
| Razer Abyssus 2014                            |  1532:0042  |
| Razer DeathAdder Chroma                       |  1532:0043  |
| Razer Mamba (Wired)                           |  1532:0044  |
| Razer Mamba (Wireless)                        |  1532:0045  |
| Razer Mamba Tournament Edition                |  1532:0046  |
| Razer Orochi (Wired)                          |  1532:0048  |
| Razer Diamondback Chroma                      |  1532:004C  |
| Razer DeathAdder 2000                         |  1532:004F  |
| Razer Naga Hex V2                             |  1532:0050  |
| Razer Naga Chroma                             |  1532:0053  |
| Razer DeathAdder 3500                         |  1532:0054  |
| Razer Lancehead (Wired)                       |  1532:0059  |
| Razer Lancehead (Wireless)                    |  1532:005A  |
| Razer Abyssus V2                              |  1532:005B  |
| Razer DeathAdder Elite                        |  1532:005C  |
| Razer Abyssus 2000                            |  1532:005E  |
| Razer Lancehead Tournament Edition            |  1532:0060  |
| Razer Atheris (Receiver)                      |  1532:0062  |
| Razer Basilisk                                |  1532:0064  |
| Razer Naga Trinity                            |  1532:0067  |
| Razer Abyssus Elite (D.Va Edition)            |  1532:006A  |
| Razer Abyssus Essential                       |  1532:006B  |
| Razer Mamba Elite (Wired)                     |  1532:006C  |
| Razer DeathAdder Essential                    |  1532:006E  |
| Razer Lancehead Wireless (Receiver)           |  1532:006F  |
| Razer Lancehead Wireless (Wired)              |  1532:0070  |
| Razer DeathAdder Essential (White Edition)    |  1532:0071  |
| Razer Mamba Wireless (Receiver)               |  1532:0072  |
| Razer Mamba Wireless (Wired)                  |  1532:0073  |
| Razer Viper                                   |  1532:0078  |
| Razer Viper Ultimate (Wired)                  |  1532:007A  |
| Razer Viper Ultimate (Wireless)               |  1532:007B  |
| Razer DeathAdder V2 Pro (Wired)               |  1532:007C  |
| Razer DeathAdder V2 Pro (Wireless)            |  1532:007D  |
| Razer Basilisk X HyperSpeed                   |  1532:0083  |
| Razer DeathAdder V2                           |  1532:0084  |
| Razer Basilisk V2                             |  1532:0085  |
| Razer Basilisk Ultimate (Wired)               |  1532:0086  |
| Razer Basilisk Ultimate (Reciever)            |  1532:0088  |
| Razer Viper Mini                              |  1532:008A  |
| Razer DeathAdder V2 Mini                      |  1532:008C  |
| Razer Naga Left-Handed Edition                |  1532:008D  |

### Mousemats
| Device                                        | USB VID:PID |
| --------------------------------------------- | ----------- |
| Razer Firefly Hyperflux                       |  1532:0068  |
| Razer Firefly                                 |  1532:0C00  |
| Razer Goliathus                               |  1532:0C01  |
| Razer Goliathus Extended                      |  1532:0C02  |
| Razer Firefly v2                              |  1532:0C04  |

### Headsets
| Device                                        | USB VID:PID |
| --------------------------------------------- | ----------- |
| Razer Kraken 7.1                              |  1532:0501  |
| Razer Kraken 7.1 Chroma                       |  1532:0504  |
| Razer Kraken 7.1                              |  1532:0506  |
| Razer Kraken 7.1 V2                           |  1532:0510  |
| Razer Kraken Ultimate                         |  1532:0527  |
| Razer Kraken Kitty Edition                    |  1532:0F19  |

### Misc
| Device                                        | USB VID:PID |
| --------------------------------------------- | ----------- |
| Razer Mouse Dock                              |  1532:007E  |
| Razer Nostromo                                |  1532:0111  |
| Razer Orbweaver                               |  1532:0113  |
| Razer Tartarus                                |  1532:0201  |
| Razer Orbweaver Chroma                        |  1532:0207  |
| Razer Tartarus Chroma                         |  1532:0208  |
| Razer Core                                    |  1532:0215  |
| Razer Tartarus V2                             |  1532:022B  |
| Razer Nommo Chroma                            |  1532:0517  |
| Razer Nommo Pro                               |  1532:0518  |
| Razer Chroma Mug Holder                       |  1532:0F07  |
| Razer Base Station Chroma                     |  1532:0F08  |
| Razer Chroma Hardware Development Kit (HDK)   |  1532:0F09  |
| Razer Mouse Bungee V3 Chroma                  |  1532:0F1D  |
| Razer Base Station V2 Chroma                  |  1532:0F20  |
| Razer Charging Pad Chroma                     |  1532:0F26  |

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
* [NixOS](https://openrazer.github.io/#nixos)
* [Solus](https://openrazer.github.io/#solus)
* [Void Linux](https://openrazer.github.io/#voidlinux)
* [Red Hat / CentOS](https://openrazer.github.io/#redhat) (unofficial)

## Applications

The following applications complement and interact with this driver:

* [Polychromatic](https://github.com/polychromatic/polychromatic) - a graphical management tool and tray applet to managing Razer peripherals.
* [RazerGenie](https://github.com/z3ntu/RazerGenie) - Qt application for configuring your Razer devices under GNU/Linux.
* [razerCommander](https://github.com/GabMus/razerCommander) - Simple GUI written in Gtk3
* [Snake](http://bithatch.co.uk/snake.html) - a stylised tool and tray applet for configuring Razer devices on Linux, written in Java.
* [Chroma Feedback](https://github.com/redaxmedia/chroma-feedback) - Turn your Razer keyboard, mouse or headphone into a extreme feedback device

## Contributions

You can donate to [@terrycain](https://github.com/terrycain) with PayPal to terry@terrys-home.co.uk .

---

The project is licensed under the GPL and is not officially endorsed by [Razer, Inc](http://www.razerzone.com/).

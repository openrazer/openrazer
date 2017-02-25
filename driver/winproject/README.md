# ChromaDLL

Open ChromaDLL.sln in VS2015
Build appropriate version (64 bit debug version is default)

Obtain the Zadig USB driver tool (to automate driver installation)

Start Zadig.

Select Options->List all devices (This queries all of the USB devices connected to your system)

The dropdown should now contain devices starting with "Razer..."  Select one of you Razer devices and select the last interface in the group.
Example:
Razer DeathAdder Elite (Interface 0)
Razer DeathAdder Elite (Interface 1)
Razer DeathAdder Elite (Interface 2)-> Select this one for instance

Select "WinUSB (v6.1.7600.16385)" in the Driver field
then select "Install Driver" from the DropDown Button, then press the button "Replace Driver"

This will install the WinUSB driver on that interface.  Don't pick interface 0 or interface 1 as those are utilized by the OS's HID driver for normal keyboard and mouse operations.

This project contains a main.cpp file which tests the various Chroma API calls on all Chroma devices found.

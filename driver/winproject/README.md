# ChromaDLL

Open ChromaDLL.sln in VS2015
Build appropriate version (64 bit debug version is default)

Filter Driver Installation - you need to do this prior to building the DLL otherwise you get a popup message from VS indicating that the libusb DLL is missing.

Obtain the Zadig USB driver tool (to automate filter driver installation)
(-or- ... use the tools within the libusb-win32 project)

Start Zadig.

Select Options->List all devices (This queries all of the USB devices connected to your system)

The dropdown should now contain devices starting with "Razer..."  Select one of you Razer devices and select the last interface in the group.
Example:
Razer DeathAdder Elite (Interface 0)
Razer DeathAdder Elite (Interface 1)
Razer DeathAdder Elite (Interface 2)-> Select this one for instance

Select "libusb-win32 (v1.2.6.0)" in the Driver field
then select "Install Driver" from the DropDown Button, then press the button "Replace Driver"

Remove it completely with Device Manager from the "libusb-win32 devices" group (Checking "delete the driver software for this device"), then right-click the top node on the device manager tree and select scan for hardware changes.  

After this, the libusb0.dll should remain in system32 and syswow64.

Next, manually copy the "install-filter.exe" to the c:\windows directory.  This file can be found in "c:\users\your.profile\usb_driver\amd64\" (The previous instruction for zadig with libusb-win32 creates this directory by default)

Next, select "libusb-win32 (v1.2.6.0)" in the Driver field
then select "Install Filter Driver" in the "Replace Driver" Dropdown Button and press the button to do the install.

This will install a filter driver that will allow the normal Razer Driver to still function while send Chroma commands from another program.


*zadig has an issue such that if you try to install a filter driver without ever having installed libusb-win32 with the "Install Driver" option, then you will receive an error indicating that window cannot find install-filter.exe.
In order to work around this, I created this procedure to install libusb-win32 on any hid device (not the main one you are using to interact with the computer) on an interface that is non-zero.

I don't like this workaround either, but it works and if anyone can come up with a better one, or even perhaps a way to use winusb instead if libusb-win32 or some other method of automating the above, I would like to update this.  You may even have better luck with the original libusb-win32 project found on sourceforge at https://sourceforge.net/projects/libusb-win32/


This project contains a main.cpp file which tests the various Chroma API calls on all Chroma devices found.

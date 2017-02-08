# ChromaDLL

Open ChromaDLL.sln in VS2015
Build appropriate version (64 bit debug version is default)

Filter Driver Installation (you may also need to do this prior to building the DLL if you get the popup message from VS indicating that the libusb DLL is missing)
*zadig has an issue such that if you try to install a filter driver without ever having installed libusb-win32 with the "Install Driver" option, then you will receive an error indicating that window cannot find install-filter.exe.
In order to work around this, install libusb-win32 with the install driver option on any hid device (not the main one you are using to interact with the computer) on an interface that is non-zero, then remove it completely with Device Manager from the "libusb-win32 devices" group (Checking "delete the driver software for this device"), then right-click the top node on the device manager tree and select scan for hardware changes.  AFter this, the libusb0.dll should remain in system32 and syswow64.  Next, manually copy the "install-filter.exe" to the c:\windows directory (this file is found in the same directory as I included zadig_2.2.exe under 'usb_driver\amd64' or oif you don't use the ini file I included, under your user profile directory).  I don't like this workaround either, but it works and if anyone can come up with a better one, or even perhaps a way to use winusb in my code (winusb is not supposed to require installation of any additional drivers), I would like to update this.  You may even have better luck with the original libusb-win32 project found on sourceforge at https://sourceforge.net/projects/libusb-win32/

Use the Zadig tool(or you can create inf files from tools included with libusb-win32 project)
Options->List all devices (This queries all of the USB devices connected to your system)
The dropdown should now contain devices starting with "Razer..."

for each device:
Select "libusb-win32 (v1.2.6.0)" in the Driver field
then select "Install Filter Driver" in the "Replace Driver" Dropdown Button.
Then press the button to do the install.
This should install a filter driver that will
(You may need to install the filter driver libusb0.dll in c:\windows\syswow64 and c:\windows\system32 prior
more detailed instructions to follow after I reproduce this on another clean box)

Copy appropriate DLL (ChromaDLL.dll or ChromaDLL64.dll) from debug/release/x64/x86 directory into winprojectexample
(I need to work on this directory structure - MS project defaults and cleans seem kind of cluttery and inconsistant)

Open ChromaExample.sln in VS2015
This contains a main.cpp file which (using the DLL) muticolors all devices found in a 50ms loop using custom/frame calls

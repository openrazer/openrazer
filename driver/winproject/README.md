# ChromaDLL

Open ChromaDLL.sln in VS2015
Build appropriate version (64 bit debug version is default)

Use zadig_2.2.exe (or you can create inf files from tools included with libusb-win32 project)
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

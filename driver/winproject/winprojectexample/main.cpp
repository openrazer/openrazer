#include <set>
#include <stdio.h>
#include <tchar.h>
#include <wtypes.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>

#ifdef _WIN64
#define CHROMALINUXDLL        _T("ChromaDLL64.dll")
#elif WIN32
#define CHROMALINUXDLL        _T("ChromaDLL.dll")
#endif

static const COLORREF testColor[] = {
		RGB(0x00,0x00,0xFF)
	  , RGB(0x00,0xFF,0x00)
	  , RGB(0xFF,0x00,0x00)
	  , RGB(0xFF,0xFF,0x00)
	  , RGB(0xFF,0x00,0xFF)
	  , RGB(0x00,0xFF,0xFF)};

static void colorize(std::set<struct device*> deviceKeyboards, unsigned int maxRow, unsigned int maxCol, unsigned int offset, struct device_attribute* frame, struct device_attribute* effect) {
	char* buf = new char[3 * maxCol + 4];
	buf[1] = 0;
	buf[2] = maxCol - 1;
	for (struct device* device : deviceKeyboards) {
		for (unsigned int row = 0; row < maxRow; row++) {
			buf[0] = row;
			for (unsigned int col = 0; col < maxCol; col++) {
				unsigned long color = testColor[(row*maxCol + col + offset) % _countof(testColor)];
				buf[3 * col + 3] = (char)(color & 0x0000FF);
				buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
				buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
			}
			frame->store(device, NULL, (const char*)&buf[0], 3 * maxCol + 4 - 1);
		}
		effect->store(device, NULL, 0, 0);
	}
	delete[] buf;
}

int main(int argc, char **argv) {
	printf("Press enter to continue...");
	getc(stdin);
	printf("\n");

	HMODULE chromaLinuxModule = LoadLibrary(CHROMALINUXDLL);
	if (chromaLinuxModule == nullptr)
		return 0;

	// map DLL calls
	typedef long(*INITRAZERDRIVER)(struct hid_device** hdev);
	
	INITRAZERDRIVER init_razer_kbd_driver = init_razer_kbd_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_kbd_driver"));
	struct device_attribute* devkbd_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_custom"));
	struct device_attribute* devkbd_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_custom_frame"));

	INITRAZERDRIVER init_razer_firefly_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_firefly_driver"));
	struct device_attribute* devfirefly_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_custom"));
	struct device_attribute* devfirefly_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_custom_frame"));

	INITRAZERDRIVER init_razer_mouse_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_mouse_driver"));
	struct device_attribute* devmouse_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_custom"));
	struct device_attribute* devmouse_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_custom_frame"));

	INITRAZERDRIVER init_razer_mug_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_mug_driver"));
	struct device_attribute* devmug_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_custom"));
	struct device_attribute* devmug_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_custom_frame"));

	INITRAZERDRIVER init_razer_kraken_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_kraken_driver"));
	//struct device_attribute* devkraken_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_custom"));
	//struct device_attribute* devkraken_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_custom_frame"));

	INITRAZERDRIVER init_razer_core_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_core_driver"));
	struct device_attribute* devcore_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_custom"));
	struct device_attribute* devcore_attr_matrix_custom_frame =  (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_custom_frame"));

	typedef void(*CLOSE)(struct device* dev);
	CLOSE close = reinterpret_cast<CLOSE>(GetProcAddress(chromaLinuxModule , "close"));

	// call init stuff
	typedef void(*INIT)(void);
	INIT init = reinterpret_cast<INIT>(GetProcAddress(chromaLinuxModule, "init"));
	if (init)
		init();

	struct hid_device* hdev;
	unsigned int num;

	hdev = NULL;
	std::set<struct device*> deviceKeyboards;
	num = init_razer_kbd_driver(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceKeyboards.insert(&hdev[i].dev);

	hdev = NULL;
	std::set<struct device*> deviceMice;
	num = init_razer_mouse_driver(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceMice.insert(&hdev[i].dev);

	hdev = NULL;
	std::set<struct device*> deviceFireflies;
	num = init_razer_firefly_driver(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceFireflies.insert(&hdev[i].dev);

	hdev = NULL;
	std::set<struct device*> deviceMugs;
	num = init_razer_mug_driver(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceMugs.insert(&hdev[i].dev);

	hdev = NULL;
	std::set<struct device*> deviceCores;
	num = init_razer_core_driver(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceCores.insert(&hdev[i].dev);

	hdev = NULL;
	std::set<struct device*> deviceKrakens;
	num = init_razer_kraken_driver(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceKrakens.insert(&hdev[i].dev);

	for(;;)
	for (unsigned int offset = 0; offset < _countof(testColor); offset++) {
		colorize(deviceKeyboards, 6, 25, offset, devkbd_attr_matrix_custom_frame, devkbd_attr_matrix_effect_custom);
		colorize(deviceMice, 1, 25, offset, devmouse_attr_matrix_custom_frame, devmouse_attr_matrix_effect_custom);
		colorize(deviceFireflies, 1, 15, offset, devfirefly_attr_matrix_custom_frame, devfirefly_attr_matrix_effect_custom);
		colorize(deviceMugs, 1, 15, offset, devmug_attr_matrix_custom_frame, devmug_attr_matrix_effect_custom);
		colorize(deviceCores, 1, 25, offset, devcore_attr_matrix_custom_frame, devcore_attr_matrix_effect_custom);
		//colorize(deviceKrakens, 9, 5, offset, devkraken_attr_matrix_custom_frame, devkraken_attr_matrix_effect_custom);
		Sleep(50);
	}

	for (struct device* device : deviceKeyboards) close(device);
	for (struct device* device : deviceMice) close(device);
	for (struct device* device : deviceFireflies) close(device);
	for (struct device* device : deviceMugs) close(device);
	for (struct device* device : deviceCores) close(device);
	for (struct device* device : deviceKrakens) close(device);
	
	return 0;
}

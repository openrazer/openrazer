#include <set>
#include <stdio.h>
#include <tchar.h>
#include <wtypes.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>

#define USB_VENDOR_ID_RAZER 0x1532
#define USB_DEVICE_ID_RAZER_CHROMA_MUG 0x0f07

#define USB_DEVICE_ID_RAZER_IMPERATOR 0x002F
#define USB_DEVICE_ID_RAZER_OUROBOROS 0x0032
#define USB_DEVICE_ID_RAZER_ABYSSUS 0x0042
#define USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA 0x0043
#define USB_DEVICE_ID_RAZER_MAMBA_WIRED 0x0044
#define USB_DEVICE_ID_RAZER_MAMBA_WIRELESS 0x0045
#define USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED 0x0046
#define USB_DEVICE_ID_RAZER_OROCHI_CHROMA 0x0048
#define USB_DEVICE_ID_RAZER_NAGA_HEX_V2 0x0050
#define USB_DEVICE_ID_RAZER_DEATHADDER_ELITE 0x005C
#define USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA 0x004C

#define RAZER_MAMBA_ROW_LEN 15          // 0 => 14
#define RAZER_MAMBA_TE_ROW_LEN 16       // 0 => 15
#define RAZER_DIAMONDBACK_ROW_LEN 21    // 0 => 20

#define USB_DEVICE_ID_RAZER_KRAKEN_V2 0x0510
#define USB_DEVICE_ID_RAZER_KRAKEN 0x0504

#define USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012 0x010D
#define USB_DEVICE_ID_RAZER_ANANSI 0x010F
#define USB_DEVICE_ID_RAZER_ORBWEAVER 0x0113
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013 0x011A
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL 0x011B
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA 0x0203
#define USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA 0x0204
#define USB_DEVICE_ID_RAZER_BLADE_STEALTH 0x0205
#define USB_DEVICE_ID_RAZER_TARTARUS_CHROMA 0x0208
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE 0x0209
#define USB_DEVICE_ID_RAZER_BLADE_QHD 0x020F
#define USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016 0x0210
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016 0x0214
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA 0x0216
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_X_ULTIMATE 0x0217
#define USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA_TE 0x021a
#define USB_DEVICE_ID_RAZER_ORNATA_CHROMA 0x021e
#define USB_DEVICE_ID_RAZER_ORNATA 0x021f
#define USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016 0x0220

#define RAZER_BLACKWIDOW_CHROMA_WAVE_DIRECTION_LEFT 2
#define RAZER_BLACKWIDOW_CHROMA_WAVE_DIRECTION_RIGHT 1

#define RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT 0x0A

#define RAZER_BLACKWIDOW_CHROMA_EFFECT_NONE 0
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_WAVE 1
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_REACTIVE 2
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_BREATH 3
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_SPECTRUM 4
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_CUSTOM 5 // draw frame 
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_STATIC 6
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_CLEAR_ROW 8

#define RAZER_BLACKWIDOW_ULTIMATE_2016_EFFECT_STARLIGHT 0x19

#define RAZER_BLACKWIDOW_CHROMA_EFFECT_SET_KEYS 9 //update profile needs to be called after setting keys to reflect changes
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_RESET 10
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN 11
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN2 12
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN3 13
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN4 14

#define RAZER_BLACKWIDOW_CHROMA_ROW_LEN 0x16
#define RAZER_BLACKWIDOW_CHROMA_ROWS_NUM 6

#define RAZER_STEALTH_ROW_LEN 0x10
#define RAZER_STEALTH_ROWS_NUM 6

#define USB_DEVICE_ID_RAZER_FIREFLY 0x0C00

#define RAZER_FIREFLY_WAVE_DIRECTION_ACW 2
#define RAZER_FIREFLY_WAVE_DIRECTION_CW 1

#define RAZER_FIREFLY_CHANGE_EFFECT 0x0A

#define RAZER_FIREFLY_EFFECT_NONE 0
#define RAZER_FIREFLY_EFFECT_WAVE 1
#define RAZER_FIREFLY_EFFECT_REACTIVE 2 // Didn't get this working
#define RAZER_FIREFLY_EFFECT_BREATH 3
#define RAZER_FIREFLY_EFFECT_SPECTRUM 4
#define RAZER_FIREFLY_EFFECT_CUSTOM 5
#define RAZER_FIREFLY_EFFECT_STATIC 6
#define RAZER_FIREFLY_EFFECT_CLEAR_ROW 8

#define RAZER_FIREFLY_ROW_LEN 0x0F
#define RAZER_FIREFLY_ROWS_NUM 1

#define USB_DEVICE_ID_RAZER_CORE 0x0215

#define RAZER_CORE_WAVE_DIRECTION_ACW 2
#define RAZER_CORE_WAVE_DIRECTION_CW 1

#define RAZER_CORE_CHANGE_EFFECT 0x0A

#define RAZER_CORE_EFFECT_NONE 0
#define RAZER_CORE_EFFECT_WAVE 1
#define RAZER_CORE_EFFECT_REACTIVE 2 // Didn't get this working
#define RAZER_CORE_EFFECT_BREATH 3
#define RAZER_CORE_EFFECT_SPECTRUM 4
#define RAZER_CORE_EFFECT_CUSTOM 5
#define RAZER_CORE_EFFECT_STATIC 6
#define RAZER_CORE_EFFECT_CLEAR_ROW 8

#define RAZER_CORE_ROW_LEN 0x0F
#define RAZER_CORE_ROWS_NUM 1

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

static const char* testReactive[] = {
		"\x00\xFF\x00\x00"
	,	"\x20\xFF\xFF\x00"
	,	"\x30\x00\xFF\x00"};

static const char* testBrightness[] = {
		"0"
	  , "100"
	  , "200"};

static void colorize(std::set<struct device*> devices, unsigned int maxRow, unsigned int maxCol, unsigned int offset, struct device_attribute frame, struct device_attribute effect) {
	char* buf = new char[3 * maxCol + 4];
	buf[1] = 0;
	buf[2] = maxCol - 1;
	for (struct device* device : devices) {
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_ANANSI:
			//case USB_DEVICE_ID_RAZER_DEATHSTALKER_ULTIMATE:
				//break;
			default:
				for (unsigned int row = 0; row < maxRow; row++) {
					buf[0] = row;
					for (unsigned int col = 0; col < maxCol; col++) {
						unsigned long color = testColor[(row*maxCol + col + offset) % _countof(testColor)];
						buf[3 * col + 3] = (char)(color & 0x0000FF);
						buf[3 * col + 4] = (char)((color & 0x00FF00) >> 8);
						buf[3 * col + 5] = (char)((color & 0xFF0000) >> 16);
					}
					frame.store(device, NULL, (const char*)&buf[0], 3 * maxCol + 4 - 1);
				}
				effect.store(device, NULL, 0, 0);
		}
		//printf("%s\n", effect.name);
	}
	delete[] buf;
}

static unsigned char red(COLORREF color) {
	return (char)(color & 0x0000FF);
}

static unsigned char green(COLORREF color) {
	return (char)((color & 0x00FF00) >> 8);
}

static unsigned char blue(COLORREF color) {
	return (char)((color & 0xFF0000) >> 16);
}

static void staticEffect(struct device* device, unsigned long color, struct device_attribute effect) {
	char buf[4] = "\x00\x00\x00";
	buf[0] = red(color);
	buf[1] = green(color);
	buf[2] = blue(color);
	effect.store(device, NULL, (const char*)&buf[0], 3);
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
	struct device_attribute* devkbd_attr_matrix_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_brightness"));
	struct device_attribute* devkbd_attr_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_none"));
	struct device_attribute* devkbd_attr_matrix_effect_static = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_static"));
	struct device_attribute* devkbd_attr_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_spectrum"));
	struct device_attribute* devkbd_attr_matrix_effect_reactive = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_reactive"));

	INITRAZERDRIVER init_razer_firefly_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_firefly_driver"));
	struct device_attribute* devfirefly_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_custom"));
	struct device_attribute* devfirefly_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_custom_frame"));
	struct device_attribute* devfirefly_attr_matrix_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_brightness"));
	struct device_attribute* devfirefly_attr_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_none"));
	struct device_attribute* devfirefly_attr_matrix_effect_static = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_static"));
	struct device_attribute* devfirefly_attr_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_spectrum"));
	struct device_attribute* devfirefly_attr_matrix_effect_reactive = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_reactive"));
	
	INITRAZERDRIVER init_razer_mouse_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_mouse_driver"));
	struct device_attribute* devmouse_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_custom"));
	struct device_attribute* devmouse_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_custom_frame"));
	struct device_attribute* devmouse_attr_matrix_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_brightness"));
	struct device_attribute* devmouse_attr_logo_led_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_led_brightness"));
	struct device_attribute* devmouse_attr_scroll_led_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_led_brightness"));
	struct device_attribute* devmouse_attr_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_none"));
	struct device_attribute* devmouse_attr_logo_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_none"));
	struct device_attribute* devmouse_attr_scroll_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_none"));
	struct device_attribute* devmouse_attr_matrix_effect_static = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_static"));
	struct device_attribute* devmouse_attr_logo_matrix_effect_static = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_static"));
	struct device_attribute* devmouse_attr_scroll_matrix_effect_static = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_static"));
	struct device_attribute* devmouse_attr_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_spectrum"));
	struct device_attribute* devmouse_attr_logo_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_spectrum"));
	struct device_attribute* devmouse_attr_scroll_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_spectrum"));
	struct device_attribute* devmouse_attr_matrix_effect_reactive = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_reactive"));
	struct device_attribute* devmouse_attr_logo_matrix_effect_reactive = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_reactive"));
	struct device_attribute* devmouse_attr_scroll_matrix_effect_reactive = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_reactive"));

	INITRAZERDRIVER init_razer_mug_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_mug_driver"));
	struct device_attribute* devmug_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_custom"));
	struct device_attribute* devmug_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_custom_frame"));
	struct device_attribute* devmug_attr_matrix_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_brightness"));
	struct device_attribute* devmug_attr_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_none"));
	struct device_attribute* devmug_attr_matrix_effect_static = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_static"));
	struct device_attribute* devmug_attr_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_spectrum"));

	INITRAZERDRIVER init_razer_kraken_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_kraken_driver"));
	//struct device_attribute* devkraken_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_custom"));
	//struct device_attribute* devkraken_attr_matrix_custom_frame = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_custom_frame"));
	struct device_attribute* devkraken_attr_matrix_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_brightness"));
	struct device_attribute* devkraken_attr_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_none"));
	struct device_attribute* devkraken_attr_matrix_effect_static = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_static"));
	struct device_attribute* devkraken_attr_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_spectrum"));

	INITRAZERDRIVER init_razer_core_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_core_driver"));
	struct device_attribute* devcore_attr_matrix_effect_custom = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_custom"));
	struct device_attribute* devcore_attr_matrix_custom_frame =  (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_custom_frame"));
	struct device_attribute* devcore_attr_matrix_brightness = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_brightness"));
	struct device_attribute* devcore_attr_matrix_effect_none = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_none"));
	struct device_attribute* devcore_attr_matrix_effect_static =  (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_static"));
	struct device_attribute* devcore_attr_matrix_effect_spectrum = (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_spectrum"));
	struct device_attribute* devcore_attr_matrix_effect_reactive= (struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_reactive"));

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
	std::set<struct device*> deviceFireflies;
	num = init_razer_firefly_driver(&hdev);
	for (unsigned int i = 0; i < num; i++)
		deviceFireflies.insert(&hdev[i].dev);

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

	printf("Press enter to test spectrum effects...");
	getc(stdin);
	printf("\n");

	for (struct device* device : deviceKeyboards) 
		devkbd_attr_matrix_effect_spectrum->store(device, NULL, 0, 0);
	for (struct device* device : deviceMice) 
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
				devmouse_attr_logo_matrix_effect_spectrum->store(device, NULL, 0, 0);
				devmouse_attr_scroll_matrix_effect_spectrum->store(device, NULL, 0, 0);
				break;
			default:
				devmouse_attr_matrix_effect_spectrum->store(device, NULL, 0, 0);
		}
	for (struct device* device : deviceFireflies) 
		devfirefly_attr_matrix_effect_spectrum->store(device, NULL, 0, 0);
	for (struct device* device : deviceMugs) 
		devmug_attr_matrix_effect_spectrum->store(device, NULL, 0, 0);
	for (struct device* device : deviceCores) 
		devcore_attr_matrix_effect_spectrum->store(device, NULL, 0, 0);
	for (struct device* device : deviceKrakens) 
		devkraken_attr_matrix_effect_spectrum->store(device, NULL, 0, 0);

	for (int i = 0; i < _countof(testBrightness); i++) {
		printf("Press enter to test brightness level %s ...", testBrightness[i]);
		getc(stdin);
		printf("\n");

		for (struct device* device : deviceKeyboards) 
			devkbd_attr_matrix_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		for (struct device* device : deviceMice)
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
					devmouse_attr_logo_led_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
					devmouse_attr_scroll_led_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
					break;
				default:
					devmouse_attr_matrix_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
			}
		for (struct device* device : deviceFireflies) 
			devfirefly_attr_matrix_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		for (struct device* device : deviceMugs) 
			devmug_attr_matrix_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		for (struct device* device : deviceCores) 
			devcore_attr_matrix_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		for (struct device* device : deviceKrakens) 
			devkraken_attr_matrix_brightness->store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
	}

	printf("Press enter to test none (turn everything off)...");
	getc(stdin);
	printf("\n");

	for (struct device* device : deviceKeyboards) 
		devkbd_attr_matrix_effect_none->store(device, NULL, 0, 0);
	for (struct device* device : deviceMice)
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
				devmouse_attr_logo_matrix_effect_none->store(device, NULL, 0, 0);
				devmouse_attr_scroll_matrix_effect_none->store(device, NULL, 0, 0);
				break;
			default:
				devmouse_attr_matrix_effect_none->store(device, NULL, 0, 0);
		}
	for (struct device* device : deviceFireflies) 
		devfirefly_attr_matrix_effect_none->store(device, NULL, 0, 0);
	for (struct device* device : deviceMugs) 
		devmug_attr_matrix_effect_none->store(device, NULL, 0, 0);
	for (struct device* device : deviceCores) 
		devcore_attr_matrix_effect_none->store(device, NULL, 0, 0);
	for (struct device* device : deviceKrakens) 
		devkraken_attr_matrix_effect_none->store(device, NULL, 0, 0);

	printf("Press enter to test reactive effects...");
	getc(stdin);
	printf("\n");

	for (int i = 0; i < _countof(testReactive); i++) {
		for (struct device* device : deviceKeyboards) 
			devkbd_attr_matrix_effect_reactive->store(device, NULL, testReactive[i], 4);
		for (struct device* device : deviceMice) 
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
					devmouse_attr_logo_matrix_effect_reactive->store(device, NULL, testReactive[i], 4);
					devmouse_attr_scroll_matrix_effect_reactive->store(device, NULL, testReactive[i], 4);
					break;
				default:
					devmouse_attr_matrix_effect_reactive->store(device, NULL, testReactive[i], 4);
			}
		for (struct device* device : deviceFireflies) 
			devfirefly_attr_matrix_effect_reactive->store(device, NULL, testReactive[i], 4);
		for (struct device* device : deviceCores) 
			devcore_attr_matrix_effect_reactive->store(device, NULL, testReactive[i], 4);

		printf("Speed %02X Color RGB(%02X,%02X,%02X) sent.  Press enter to test next color...",(unsigned char)testReactive[i][0], (unsigned char)testReactive[i][1], (unsigned char)testReactive[i][2], (unsigned char)testReactive[i][3]);
		getc(stdin);
		printf("\n");
	}

	printf("Press enter to test static effects (loop through 6 colors)...");
	getc(stdin);
	printf("\n");

	for (int i = 0; i < _countof(testColor); i++) {
		for (struct device* device : deviceKeyboards) 
			staticEffect(device, testColor[i], *devkbd_attr_matrix_effect_static);
		for (struct device* device : deviceMice) 
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
					staticEffect(device, testColor[i], *devmouse_attr_logo_matrix_effect_static);
					staticEffect(device, testColor[i], *devmouse_attr_scroll_matrix_effect_static);
					break;
				default:
					staticEffect(device, testColor[i], *devmouse_attr_matrix_effect_static);
			}
		for (struct device* device : deviceFireflies) 
			staticEffect(device, testColor[i], *devfirefly_attr_matrix_effect_static);
		for (struct device* device : deviceMugs) 
			staticEffect(device, testColor[i], *devmug_attr_matrix_effect_static);
		for (struct device* device : deviceCores) 
			staticEffect(device, testColor[i], *devcore_attr_matrix_effect_static);
		for (struct device* device : deviceKrakens) 
			staticEffect(device, testColor[i], *devkraken_attr_matrix_effect_static);
		printf("Color RGB(%02X,%02X,%02X) sent.  Press enter to test next color...",red(testColor[i]), green(testColor[i]), blue(testColor[i]));
		getc(stdin);
		printf("\n");
	}

	printf("Press enter to test custom effects...");
	getc(stdin);
	printf("\n");

	for(int i = 0;i < 10;i++)
	for (unsigned int offset = 0; offset < _countof(testColor); offset++) {
		colorize(deviceKeyboards, 6, 25, offset, *devkbd_attr_matrix_custom_frame, *devkbd_attr_matrix_effect_custom);
		colorize(deviceMice, 1, 25, offset, *devmouse_attr_matrix_custom_frame, *devmouse_attr_matrix_effect_custom);
		colorize(deviceFireflies, 1, 15, offset, *devfirefly_attr_matrix_custom_frame, *devfirefly_attr_matrix_effect_custom);
		colorize(deviceMugs, 1, 15, offset, *devmug_attr_matrix_custom_frame, *devmug_attr_matrix_effect_custom);
		colorize(deviceCores, 1, 25, offset, *devcore_attr_matrix_custom_frame, *devcore_attr_matrix_effect_custom);
		//colorize(deviceKrakens, 9, 5, offset, *devkraken_attr_matrix_custom_frame, *devkraken_attr_matrix_effect_custom);
		Sleep(50);
	}

	printf("Press enter to close everything...");
	getc(stdin);
	printf("\n");

	for (struct device* device : deviceKeyboards) close(device);
	for (struct device* device : deviceMice) close(device);
	for (struct device* device : deviceFireflies) close(device);
	for (struct device* device : deviceMugs) close(device);
	for (struct device* device : deviceCores) close(device);
	for (struct device* device : deviceKrakens) close(device);

	return 0;
}
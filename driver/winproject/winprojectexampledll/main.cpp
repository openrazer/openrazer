#include <set>
#include <stdio.h>
#include <tchar.h>
#include <wtypes.h>

#include <linux/kernel.h>
#include <defines.h>
#include <linux/module.h>
#include <linux/hid.h>

#ifdef DLL_INTERNAL
// Hack to turn Linux device macros into API calls
#define DEVICE_ATTRH1(_device,_name, _mode, _show, _store)	\
	struct device_attribute dev1##_device##_attr_##_name; \
	DLL_INTERNAL struct device_attribute dev##_device##_attr_##_name;

#define module_hid_driverh(hdr) \
unsigned int init_##hdr## (struct hid_device** hdevo);

extern "C" {
#include "winproject\razerinit.h"
#include "razerchromacommon.h"
#include "razerkbd_driver.h"
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTRH1(kbd, _name, _mode, _show, _store)
static DEVICE_ATTR(game_led_state,          0660, razer_attr_read_mode_game,                  razer_attr_write_mode_game);
static DEVICE_ATTR(macro_led_state,         0660, razer_attr_read_mode_macro,                 razer_attr_write_mode_macro);
static DEVICE_ATTR(macro_led_effect,        0660, razer_attr_read_mode_macro_effect,          razer_attr_write_mode_macro_effect);
static DEVICE_ATTR(logo_led_state,          0220, NULL,                                       razer_attr_write_set_logo);
static DEVICE_ATTR(profile_led_red,         0660, razer_attr_read_tartarus_profile_led_red,   razer_attr_write_tartarus_profile_led_red);
static DEVICE_ATTR(profile_led_green,       0660, razer_attr_read_tartarus_profile_led_green, razer_attr_write_tartarus_profile_led_green);
static DEVICE_ATTR(profile_led_blue,        0660, razer_attr_read_tartarus_profile_led_blue,  razer_attr_write_tartarus_profile_led_blue);


static DEVICE_ATTR(test,                    0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_get_firmware_version,       NULL);
static DEVICE_ATTR(fn_toggle,               0220, NULL,                                       razer_attr_write_set_fn_toggle);

static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,                razer_attr_write_device_mode);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_get_serial,                 NULL);

static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_mode_none);
static DEVICE_ATTR(matrix_effect_wave,      0220, NULL,                                       razer_attr_write_mode_wave);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                       razer_attr_write_mode_spectrum);
static DEVICE_ATTR(matrix_effect_reactive,  0220, NULL,                                       razer_attr_write_mode_reactive);
static DEVICE_ATTR(matrix_effect_static,    0220, NULL,                                       razer_attr_write_mode_static);
static DEVICE_ATTR(matrix_effect_starlight, 0220, NULL,                                       razer_attr_write_mode_starlight);
static DEVICE_ATTR(matrix_effect_breath,    0220, NULL,                                       razer_attr_write_mode_breath);
static DEVICE_ATTR(matrix_effect_pulsate,   0660, razer_attr_read_mode_pulsate,               razer_attr_write_mode_pulsate);
static DEVICE_ATTR(matrix_brightness,       0660, razer_attr_read_set_brightness,             razer_attr_write_set_brightness);
static DEVICE_ATTR(matrix_effect_custom,    0220, NULL,                                       razer_attr_write_mode_custom);
static DEVICE_ATTR(matrix_custom_frame,     0220, NULL,                                       razer_attr_write_matrix_custom_frame);


static DEVICE_ATTR(key_super,               0660, razer_attr_read_key_super,                  razer_attr_write_key_super);
static DEVICE_ATTR(key_alt_tab,             0660, razer_attr_read_key_alt_tab,                razer_attr_write_key_alt_tab);
static DEVICE_ATTR(key_alt_f4,              0660, razer_attr_read_key_alt_f4,                 razer_attr_write_key_alt_f4);
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTR1(kbd, _name, _mode, _show, _store)
module_hid_driverh(razer_kbd_driver);

#include "razermouse_driver.h"
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTRH1(mouse, _name, _mode, _show, _store)
static DEVICE_ATTR(version,                   0440, razer_attr_read_version,               NULL);
static DEVICE_ATTR(firmware_version,          0440, razer_attr_read_get_firmware_version,  NULL);
static DEVICE_ATTR(test,                      0220, NULL,                                  razer_attr_write_test);
static DEVICE_ATTR(poll_rate,                 0660, razer_attr_read_poll_rate,             razer_attr_write_poll_rate);
static DEVICE_ATTR(dpi,                       0660, razer_attr_read_mouse_dpi,             razer_attr_write_mouse_dpi);
                                                                                           
static DEVICE_ATTR(device_type,               0440, razer_attr_read_device_type,           NULL);
static DEVICE_ATTR(device_mode,               0660, razer_attr_read_device_mode,           razer_attr_write_device_mode);
static DEVICE_ATTR(device_serial,             0440, razer_attr_read_get_serial,            NULL);
static DEVICE_ATTR(device_idle_time,          0220, NULL,                                  razer_attr_write_set_idle_time);
                                                                                           
static DEVICE_ATTR(charge_level,              0440, razer_attr_read_get_battery,           NULL);
static DEVICE_ATTR(charge_status,             0440, razer_attr_read_is_charging,           NULL);
static DEVICE_ATTR(charge_effect,             0220, NULL,                                  razer_attr_write_set_charging_effect);
static DEVICE_ATTR(charge_colour,             0220, NULL,                                  razer_attr_write_set_charging_colour);
static DEVICE_ATTR(charge_low_threshold,      0220, NULL,                                  razer_attr_write_set_low_battery_threshold);
                                                                                           
static DEVICE_ATTR(matrix_brightness,         0660, razer_attr_read_set_brightness,        razer_attr_write_set_brightness);
static DEVICE_ATTR(matrix_custom_frame,       0220, NULL,                                  razer_attr_write_set_key_row);
static DEVICE_ATTR(matrix_effect_none,        0220, NULL,                                  razer_attr_write_mode_none);   // Matrix
static DEVICE_ATTR(matrix_effect_custom,      0220, NULL,                                  razer_attr_write_mode_custom);   // Matrix
static DEVICE_ATTR(matrix_effect_static,      0220, NULL,                                  razer_attr_write_mode_static);   // Matrix
static DEVICE_ATTR(matrix_effect_wave,        0220, NULL,                                  razer_attr_write_mode_wave);   // Matrix
static DEVICE_ATTR(matrix_effect_spectrum,    0220, NULL,                                  razer_attr_write_mode_spectrum);   // Matrix
static DEVICE_ATTR(matrix_effect_reactive,    0220, NULL,                                  razer_attr_write_mode_reactive);   // Matrix
static DEVICE_ATTR(matrix_effect_breath,      0220, NULL,                                  razer_attr_write_mode_breath);   // Matrix


static DEVICE_ATTR(scroll_led_brightness,     0660, razer_attr_read_scroll_led_brightness, razer_attr_write_scroll_led_brightness); 
// For old-school led commands
static DEVICE_ATTR(scroll_led_state,          0660, razer_attr_read_scroll_led_state,      razer_attr_write_scroll_led_state); 
static DEVICE_ATTR(scroll_led_rgb,            0660, razer_attr_read_scroll_led_rgb,        razer_attr_write_scroll_led_rgb);
static DEVICE_ATTR(scroll_led_effect,         0660, razer_attr_read_scroll_led_effect,     razer_attr_write_scroll_led_effect);
// For "extended" matrix effects
static DEVICE_ATTR(scroll_matrix_effect_spectrum,    0220, NULL,                           razer_attr_write_scroll_mode_spectrum);
static DEVICE_ATTR(scroll_matrix_effect_reactive,    0220, NULL,                           razer_attr_write_scroll_mode_reactive);
static DEVICE_ATTR(scroll_matrix_effect_breath,      0220, NULL,                           razer_attr_write_scroll_mode_breath);
static DEVICE_ATTR(scroll_matrix_effect_static,      0220, NULL,                           razer_attr_write_scroll_mode_static);
static DEVICE_ATTR(scroll_matrix_effect_none,        0220, NULL,                           razer_attr_write_scroll_mode_none);

static DEVICE_ATTR(logo_led_brightness,       0660, razer_attr_read_logo_led_brightness,   razer_attr_write_logo_led_brightness); 
// For old-school led commands
static DEVICE_ATTR(logo_led_state,            0660, razer_attr_read_logo_led_state,        razer_attr_write_logo_led_state); 
static DEVICE_ATTR(logo_led_rgb,              0660, razer_attr_read_logo_led_rgb,          razer_attr_write_logo_led_rgb);
static DEVICE_ATTR(logo_led_effect,           0660, razer_attr_read_logo_led_effect,       razer_attr_write_logo_led_effect);
// For "extended" matrix effects
static DEVICE_ATTR(logo_matrix_effect_spectrum,    0220, NULL,                             razer_attr_write_logo_mode_spectrum);
static DEVICE_ATTR(logo_matrix_effect_reactive,    0220, NULL,                             razer_attr_write_logo_mode_reactive);
static DEVICE_ATTR(logo_matrix_effect_breath,      0220, NULL,                             razer_attr_write_logo_mode_breath);
static DEVICE_ATTR(logo_matrix_effect_static,      0220, NULL,                             razer_attr_write_logo_mode_static);
static DEVICE_ATTR(logo_matrix_effect_none,        0220, NULL,                             razer_attr_write_logo_mode_none);
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTR1(mouse, _name, _mode, _show, _store)
module_hid_driverh(razer_mouse_driver);

#include "razerkraken_driver.h"
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTRH1(kraken, _name, _mode, _show, _store)
static DEVICE_ATTR(test,                    0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_get_serial,                 NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,                razer_attr_write_device_mode);
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_get_firmware_version,       NULL);

static DEVICE_ATTR(matrix_current_effect,	0440, razer_attr_read_matrix_current_effect,      NULL);
static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_mode_none);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                       razer_attr_write_mode_spectrum);
static DEVICE_ATTR(matrix_effect_static,    0660, razer_attr_read_mode_static,                razer_attr_write_mode_static);
static DEVICE_ATTR(matrix_effect_breath,    0660, razer_attr_read_mode_breath,                razer_attr_write_mode_breath);
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTR1(kraken, _name, _mode, _show, _store)
module_hid_driverh(razer_kraken_driver);

#include "razerfirefly_driver.h"
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTRH1(firefly, _name, _mode, _show, _store)
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_get_firmware_version, NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,          NULL);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_get_serial,           NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,          razer_attr_write_device_mode);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,              NULL);
static DEVICE_ATTR(matrix_brightness,       0664, razer_attr_read_set_brightness,       razer_attr_write_set_brightness);
static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                 razer_attr_write_mode_none);
static DEVICE_ATTR(matrix_effect_wave,      0220, NULL,                                 razer_attr_write_mode_wave);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                 razer_attr_write_mode_spectrum);
static DEVICE_ATTR(matrix_effect_reactive,  0220, NULL,                                 razer_attr_write_mode_reactive);
static DEVICE_ATTR(matrix_effect_breath,    0220, NULL,                                 razer_attr_write_mode_breath);
static DEVICE_ATTR(matrix_effect_custom,    0220, NULL,                                 razer_attr_write_mode_custom);
static DEVICE_ATTR(matrix_effect_static,    0220, NULL,                                 razer_attr_write_mode_static);
static DEVICE_ATTR(matrix_custom_frame,     0220, NULL,                                 razer_attr_write_set_key_row);
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTR1(firefly, _name, _mode, _show, _store)
module_hid_driverh(razer_firefly_driver);

#include "razermug_driver.h"
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTRH1(mug, _name, _mode, _show, _store)
static DEVICE_ATTR(test,                    0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,                razer_attr_write_device_mode);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_get_serial,                 NULL);
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_get_firmware_version,       NULL);

static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_mode_none);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                       razer_attr_write_mode_spectrum);
static DEVICE_ATTR(matrix_effect_static,    0220, NULL,                                       razer_attr_write_mode_static);
static DEVICE_ATTR(matrix_effect_breath,    0220, NULL,                                       razer_attr_write_mode_breath);
static DEVICE_ATTR(matrix_effect_custom,    0220, NULL,                                       razer_attr_write_mode_custom);
static DEVICE_ATTR(matrix_effect_wave,      0220, NULL,                                       razer_attr_write_mode_wave);
static DEVICE_ATTR(matrix_effect_blinking,  0220, NULL,                                       razer_attr_write_mode_blinking);
static DEVICE_ATTR(matrix_brightness,       0660, razer_attr_read_set_brightness,             razer_attr_write_set_brightness);
static DEVICE_ATTR(matrix_custom_frame,     0220, NULL,                                       razer_attr_write_set_key_row);

static DEVICE_ATTR(is_mug_present,          0440, razer_attr_read_get_cup_state,              NULL);
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTR1(mug, _name, _mode, _show, _store)
module_hid_driverh(razer_mug_driver);

#include "razercore_driver.h"
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTRH1(core, _name, _mode, _show, _store)
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_get_firmware_version, NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,          NULL);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_get_serial,           NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,          razer_attr_write_device_mode);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,              NULL);
static DEVICE_ATTR(matrix_brightness,       0664, razer_attr_read_set_brightness,       razer_attr_write_set_brightness);
static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                 razer_attr_write_mode_none);
static DEVICE_ATTR(matrix_effect_wave,      0220, NULL,                                 razer_attr_write_mode_wave);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                 razer_attr_write_mode_spectrum);
static DEVICE_ATTR(matrix_effect_reactive,  0220, NULL,                                 razer_attr_write_mode_reactive);
static DEVICE_ATTR(matrix_effect_breath,    0220, NULL,                                 razer_attr_write_mode_breath);
static DEVICE_ATTR(matrix_effect_custom,    0220, NULL,                                 razer_attr_write_mode_custom);
static DEVICE_ATTR(matrix_effect_static,    0220, NULL,                                 razer_attr_write_mode_static);
static DEVICE_ATTR(matrix_custom_frame,     0220, NULL,                                 razer_attr_write_set_key_row);
#undef DEVICE_ATTR
#define DEVICE_ATTR(_name, _mode, _show, _store) DEVICE_ATTR1(core, _name, _mode, _show, _store)
module_hid_driverh(razer_core_driver);

}
#else
#define USB_VENDOR_ID_RAZER 0x1532
#define USB_DEVICE_ID_RAZER_CHROMA_MUG 0x0f07

#define USB_DEVICE_ID_RAZER_IMPERATOR 0x002F
#define USB_DEVICE_ID_RAZER_OUROBOROS 0x0032
#define USB_DEVICE_ID_RAZER_ABYSSUS 0x0042
#define USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA 0x0043
#define USB_DEVICE_ID_RAZER_MAMBA_WIRED 0x0044
#define USB_DEVICE_ID_RAZER_MAMBA_WIRELESS 0x0045
#define USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED 0x0046
#define USB_DEVICE_ID_RAZER_OROCHI_2013 0x0039
#define USB_DEVICE_ID_RAZER_OROCHI_CHROMA 0x0048
#define USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS 0x0021
#define USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED 0x001F
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
#define USB_DEVICE_ID_RAZER_DEATHSTALKER_ULTIMATE 0x0114
#define USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA 0x0204
#define USB_DEVICE_ID_RAZER_BLADE_STEALTH 0x0205
#define USB_DEVICE_ID_RAZER_BLADE_2015 0x011D
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
	  , "31"
	  , "63"
	  , "95"
	  , "127"
	  , "159"
	  , "191"
	  , "223"
	  , "255"};

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

static void colorize(std::set<struct device*> devices, unsigned int maxRow, unsigned int maxCol, unsigned int offset, struct device_attribute frame, struct device_attribute effect) {
	char* buf = new char[3 * maxCol + 4];
	buf[1] = 0;
	buf[2] = maxCol - 1;
	for (struct device* device : devices) {
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_ANANSI:
			case USB_DEVICE_ID_RAZER_DEATHSTALKER_ULTIMATE:
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED:
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS:
			case USB_DEVICE_ID_RAZER_OROCHI_2013:
			case USB_DEVICE_ID_RAZER_BLADE_2015:
				break;
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

int main(int argc, char **argv) {
	/*
	UsbDevice usb;
	usb.OpenDevice(0x1038, 0x1600, 0x0000);
    usb.OpenDevice(0x1770, 0xFF00, 0);

    usb.OpenDevice(0x1532, 0x0C00, 0);
    usb.OpenDevice(0x1532, 0x005C, 0);
    usb.OpenDevice(0x1532, 0x0210, 0);
	
    usb.OpenDevice(0x1532, 0x0C00, 1);
    usb.OpenDevice(0x1532, 0x005C, 1);
    usb.OpenDevice(0x1532, 0x0210, 1);

    usb.OpenDevice(0x1532, 0x0C00, 2);
    usb.OpenDevice(0x1532, 0x005C, 2);
    usb.OpenDevice(0x1532, 0x0210, 2);

    usb.OpenDevice(0x1532, 0x0C00, 3);
    usb.OpenDevice(0x1532, 0x005C, 3);
    usb.OpenDevice(0x1532, 0x0210, 3);
	*/

#ifndef DLL_INTERNAL
	printf("Press enter to load DLL...");
	getc(stdin);
	printf("\n");

	HMODULE chromaLinuxModule = LoadLibrary(CHROMALINUXDLL);
	if (chromaLinuxModule == nullptr)
		return 0;

	// map DLL calls
	typedef unsigned int(*INITRAZERDRIVER)(struct hid_device** hdev);
	
	INITRAZERDRIVER init_razer_kbd_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_kbd_driver"));
	struct device_attribute devkbd_attr_matrix_effect_custom = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_custom"));
	struct device_attribute devkbd_attr_matrix_custom_frame = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_custom_frame"));
	struct device_attribute devkbd_attr_matrix_brightness = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_brightness"));
	struct device_attribute devkbd_attr_matrix_effect_none = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_none"));
	struct device_attribute devkbd_attr_matrix_effect_static = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_static"));
	struct device_attribute devkbd_attr_matrix_effect_spectrum = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_spectrum"));
	struct device_attribute devkbd_attr_matrix_effect_reactive = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkbd_attr_matrix_effect_reactive"));

	INITRAZERDRIVER init_razer_firefly_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_firefly_driver"));
	struct device_attribute devfirefly_attr_matrix_effect_custom = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_custom"));
	struct device_attribute devfirefly_attr_matrix_custom_frame = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_custom_frame"));
	struct device_attribute devfirefly_attr_matrix_brightness = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_brightness"));
	struct device_attribute devfirefly_attr_matrix_effect_none = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_none"));
	struct device_attribute devfirefly_attr_matrix_effect_static = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_static"));
	struct device_attribute devfirefly_attr_matrix_effect_spectrum = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_spectrum"));
	struct device_attribute devfirefly_attr_matrix_effect_reactive = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devfirefly_attr_matrix_effect_reactive"));
	
	INITRAZERDRIVER init_razer_mouse_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_mouse_driver"));
	struct device_attribute devmouse_attr_matrix_effect_custom = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_custom"));
	struct device_attribute devmouse_attr_matrix_custom_frame= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_custom_frame"));
	struct device_attribute devmouse_attr_matrix_brightness= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_brightness"));
	struct device_attribute devmouse_attr_logo_led_brightness= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_led_brightness"));
	struct device_attribute devmouse_attr_scroll_led_brightness= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_led_brightness"));
	struct device_attribute devmouse_attr_matrix_effect_none= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_none"));
	struct device_attribute devmouse_attr_logo_matrix_effect_none= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_none"));
	struct device_attribute devmouse_attr_scroll_matrix_effect_none= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_none"));
	struct device_attribute devmouse_attr_matrix_effect_static= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_static"));
	struct device_attribute devmouse_attr_logo_matrix_effect_static= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_static"));
	struct device_attribute devmouse_attr_scroll_matrix_effect_static= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_static"));
	struct device_attribute devmouse_attr_matrix_effect_spectrum= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_spectrum"));
	struct device_attribute devmouse_attr_logo_matrix_effect_spectrum= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_spectrum"));
	struct device_attribute devmouse_attr_scroll_matrix_effect_spectrum= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_spectrum"));
	struct device_attribute devmouse_attr_matrix_effect_reactive= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_matrix_effect_reactive"));
	struct device_attribute devmouse_attr_logo_matrix_effect_reactive= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_logo_matrix_effect_reactive"));
	struct device_attribute devmouse_attr_scroll_matrix_effect_reactive= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_matrix_effect_reactive"));
	struct device_attribute devmouse_attr_scroll_led_effect= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_led_effect"));
	struct device_attribute devmouse_attr_scroll_led_rgb= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_led_rgb"));
	struct device_attribute devmouse_attr_scroll_led_state= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmouse_attr_scroll_led_state"));

	INITRAZERDRIVER init_razer_mug_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_mug_driver"));
	struct device_attribute devmug_attr_matrix_effect_custom= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_custom"));
	struct device_attribute devmug_attr_matrix_custom_frame= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_custom_frame"));
	struct device_attribute devmug_attr_matrix_brightness= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_brightness"));
	struct device_attribute devmug_attr_matrix_effect_none= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_none"));
	struct device_attribute devmug_attr_matrix_effect_static= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_static"));
	struct device_attribute devmug_attr_matrix_effect_spectrum= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devmug_attr_matrix_effect_spectrum"));

	INITRAZERDRIVER init_razer_kraken_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_kraken_driver"));
	//struct device_attribute devkraken_attr_matrix_effect_custom= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_custom"));
	//struct device_attribute devkraken_attr_matrix_custom_frame= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_custom_frame"));
	//struct device_attribute devkraken_attr_matrix_brightness= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_brightness"));
	struct device_attribute devkraken_attr_matrix_effect_none= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_none"));
	struct device_attribute devkraken_attr_matrix_effect_static= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_static"));
	struct device_attribute devkraken_attr_matrix_effect_spectrum= *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devkraken_attr_matrix_effect_spectrum"));

	INITRAZERDRIVER init_razer_core_driver = reinterpret_cast<INITRAZERDRIVER>(GetProcAddress(chromaLinuxModule, "init_razer_core_driver"));
	struct device_attribute devcore_attr_matrix_effect_custom = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_custom"));
	struct device_attribute devcore_attr_matrix_custom_frame = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_custom_frame"));
	struct device_attribute devcore_attr_matrix_brightness = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_brightness"));
	struct device_attribute devcore_attr_matrix_effect_none = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_none"));
	struct device_attribute devcore_attr_matrix_effect_static = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_static"));
	struct device_attribute devcore_attr_matrix_effect_spectrum = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_spectrum"));
	struct device_attribute devcore_attr_matrix_effect_reactive = *(struct device_attribute*)reinterpret_cast<void*>(GetProcAddress(chromaLinuxModule, "devcore_attr_matrix_effect_reactive"));

	typedef void(*CLOSE)(struct device* dev);
	CLOSE close = reinterpret_cast<CLOSE>(GetProcAddress(chromaLinuxModule , "close"));

	typedef void(*INIT)(void);
	INIT init = reinterpret_cast<INIT>(GetProcAddress(chromaLinuxModule, "init"));
#endif

	printf("Press enter to init usb and devices...");
	getc(stdin);
	printf("\n");

	// call init stuff
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

	printf("Press enter to start...");
	getc(stdin);
	printf("\n");

	for (struct device* device : deviceKeyboards) 
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_BLADE_2015:
				break;
			default:
				staticEffect(device, RGB(0xFF,0xFF,0xFF), devkbd_attr_matrix_effect_static);
				break;
		}
	for (struct device* device : deviceMice)
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
				staticEffect(device, RGB(0xFF,0xFF,0xFF), devmouse_attr_logo_matrix_effect_static);
				staticEffect(device, RGB(0xFF,0xFF,0xFF), devmouse_attr_scroll_matrix_effect_static);
				break;
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED:
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS:
				devmouse_attr_scroll_led_effect.store(device, NULL, "0", strlen("0")-1);
				devmouse_attr_scroll_led_rgb.store(device, NULL, "\xFF\xFF\xFF", 3);
				break;
			case USB_DEVICE_ID_RAZER_OROCHI_2013:
				devmouse_attr_scroll_led_state.store(device, NULL, "1", 1);
				break;
			default:
				staticEffect(device, RGB(0xFF,0xFF,0xFF), devmouse_attr_matrix_effect_static);
				break;
		}
	for (struct device* device : deviceFireflies) 
		staticEffect(device, RGB(0xFF,0xFF,0xFF), devfirefly_attr_matrix_effect_static);
	for (struct device* device : deviceMugs) 
		staticEffect(device, RGB(0xFF,0xFF,0xFF), devmug_attr_matrix_effect_static);
	for (struct device* device : deviceCores) 
		staticEffect(device, RGB(0xFF,0xFF,0xFF), devcore_attr_matrix_effect_static);
	for (struct device* device : deviceKrakens) 
		staticEffect(device, RGB(0xFF,0xFF,0xFF), devkraken_attr_matrix_effect_static);

	for (int i = 0; i < _countof(testBrightness); i++) {
		printf("Press enter to test brightness level %s ...", testBrightness[i]);
		getc(stdin);
		printf("\n");

		for (struct device* device : deviceKeyboards) 
			devkbd_attr_matrix_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		for (struct device* device : deviceMice)
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
					devmouse_attr_logo_led_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
					devmouse_attr_scroll_led_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
					break;
				case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED:
				case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS:
					devmouse_attr_scroll_led_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
					break;
				case USB_DEVICE_ID_RAZER_OROCHI_2013:
					break;
				default:
					devmouse_attr_matrix_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
					break;
			}
		for (struct device* device : deviceFireflies) 
			devfirefly_attr_matrix_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		for (struct device* device : deviceMugs) 
			devmug_attr_matrix_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		for (struct device* device : deviceCores) 
			devcore_attr_matrix_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
		//for (struct device* device : deviceKrakens) 
			//devkraken_attr_matrix_brightness.store(device, NULL, testBrightness[i], strlen(testBrightness[i])-1);
	}

	printf("Press enter to test none (turn everything off)...");
	getc(stdin);
	printf("\n");

	for (struct device* device : deviceKeyboards) 
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_BLADE_2015:
				break;
			default:
				devkbd_attr_matrix_effect_none.store(device, NULL, 0, 0);
				break;
		}
	for (struct device* device : deviceMice)
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED:
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS:
				devmouse_attr_scroll_led_effect.store(device, NULL, "0", strlen("0")-1);
				devmouse_attr_scroll_led_rgb.store(device, NULL, "\x00\x00\x00", 3);
				break;
			case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
				devmouse_attr_logo_matrix_effect_none.store(device, NULL, 0, 0);
				devmouse_attr_scroll_matrix_effect_none.store(device, NULL, 0, 0);
				break;
			case USB_DEVICE_ID_RAZER_OROCHI_2013:
				devmouse_attr_scroll_led_state.store(device, NULL, "0", 1);
				break;
			default:
				devmouse_attr_matrix_effect_none.store(device, NULL, 0, 0);
				break;
		}
	for (struct device* device : deviceFireflies) 
		devfirefly_attr_matrix_effect_none.store(device, NULL, 0, 0);
	for (struct device* device : deviceMugs) 
		devmug_attr_matrix_effect_none.store(device, NULL, 0, 0);
	for (struct device* device : deviceCores) 
		devcore_attr_matrix_effect_none.store(device, NULL, 0, 0);
	for (struct device* device : deviceKrakens) 
		devkraken_attr_matrix_effect_none.store(device, NULL, 0, 0);

	printf("Press enter to test spectrum effects...");
	getc(stdin);
	printf("\n");

	for (struct device* device : deviceKeyboards) 
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_BLADE_2015:
				break;
			default:
				devkbd_attr_matrix_effect_spectrum.store(device, NULL, 0, 0);
				break;
		}
	for (struct device* device : deviceMice) 
		switch (device->parent->descriptor.idProduct) {
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED:
			case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS:
				devmouse_attr_scroll_led_effect.store(device, NULL, "4", strlen("4")-1);
				break;
			case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
				devmouse_attr_logo_matrix_effect_spectrum.store(device, NULL, 0, 0);
				devmouse_attr_scroll_matrix_effect_spectrum.store(device, NULL, 0, 0);
				break;
			case USB_DEVICE_ID_RAZER_OROCHI_2013:
				break;
			default:
				devmouse_attr_matrix_effect_spectrum.store(device, NULL, 0, 0);
				break;
		}
	for (struct device* device : deviceFireflies) 
		devfirefly_attr_matrix_effect_spectrum.store(device, NULL, 0, 0);
	for (struct device* device : deviceMugs) 
		devmug_attr_matrix_effect_spectrum.store(device, NULL, 0, 0);
	for (struct device* device : deviceCores) 
		devcore_attr_matrix_effect_spectrum.store(device, NULL, 0, 0);
	for (struct device* device : deviceKrakens) 
		devkraken_attr_matrix_effect_spectrum.store(device, NULL, 0, 0);

	printf("Press enter to test reactive effects...");
	getc(stdin);
	printf("\n");

	for (int i = 0; i < _countof(testReactive); i++) {
		for (struct device* device : deviceKeyboards) 
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_BLADE_2015:
				case USB_DEVICE_ID_RAZER_DEATHSTALKER_ULTIMATE:
					break;
				default:
					devkbd_attr_matrix_effect_reactive.store(device, NULL, testReactive[i], 4);
					break;
			}
		for (struct device* device : deviceMice) 
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED:
				case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS:
				case USB_DEVICE_ID_RAZER_OROCHI_2013:
					break;
				case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
					devmouse_attr_logo_matrix_effect_reactive.store(device, NULL, testReactive[i], 4);
					devmouse_attr_scroll_matrix_effect_reactive.store(device, NULL, testReactive[i], 4);
					break;
				default:
					devmouse_attr_matrix_effect_reactive.store(device, NULL, testReactive[i], 4);
					break;
			}
		for (struct device* device : deviceFireflies) 
			devfirefly_attr_matrix_effect_reactive.store(device, NULL, testReactive[i], 4);
		for (struct device* device : deviceCores) 
			devcore_attr_matrix_effect_reactive.store(device, NULL, testReactive[i], 4);

		printf("Speed %02X Color RGB(%02X,%02X,%02X) sent.  Press enter to test next color...",(unsigned char)testReactive[i][0], (unsigned char)testReactive[i][1], (unsigned char)testReactive[i][2], (unsigned char)testReactive[i][3]);
		getc(stdin);
		printf("\n");
	}

	printf("Press enter to test static effects (loop through 6 colors)...");
	getc(stdin);
	printf("\n");

	for (int i = 0; i < _countof(testColor); i++) {
		for (struct device* device : deviceKeyboards) 
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_BLADE_2015:
					break;
				default:
					staticEffect(device, testColor[i], devkbd_attr_matrix_effect_static);
					break;
			}
		for (struct device* device : deviceMice) 
			switch (device->parent->descriptor.idProduct) {
				case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRED:
				case USB_DEVICE_ID_RAZER_NAGA_EPIC_WIRELESS:
					devmouse_attr_scroll_led_effect.store(device, NULL, "0", strlen("0")-1);
					staticEffect(device, testColor[i], devmouse_attr_scroll_led_rgb);
					break;
				case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
					staticEffect(device, testColor[i], devmouse_attr_logo_matrix_effect_static);
					staticEffect(device, testColor[i], devmouse_attr_scroll_matrix_effect_static);
					break;
				case USB_DEVICE_ID_RAZER_OROCHI_2013:
					devmouse_attr_scroll_led_state.store(device, NULL, "1", 1); // it can only be green
					break;
				default:
					staticEffect(device, testColor[i], devmouse_attr_matrix_effect_static);
			}
		for (struct device* device : deviceFireflies) 
			staticEffect(device, testColor[i], devfirefly_attr_matrix_effect_static);
		for (struct device* device : deviceMugs) 
			staticEffect(device, testColor[i], devmug_attr_matrix_effect_static);
		for (struct device* device : deviceCores) 
			staticEffect(device, testColor[i], devcore_attr_matrix_effect_static);
		for (struct device* device : deviceKrakens) 
			staticEffect(device, testColor[i], devkraken_attr_matrix_effect_static);
		printf("Color RGB(%02X,%02X,%02X) sent.  Press enter to test next color...",red(testColor[i]), green(testColor[i]), blue(testColor[i]));
		getc(stdin);
		printf("\n");
	}

	printf("Press enter to test custom effects...");
	getc(stdin);
	printf("\n");

	for(int i = 0;i < 10;i++)
	for (unsigned int offset = 0; offset < _countof(testColor); offset++) {
		colorize(deviceKeyboards, 6, 25, offset, devkbd_attr_matrix_custom_frame, devkbd_attr_matrix_effect_custom);
		colorize(deviceMice, 1, 25, offset, devmouse_attr_matrix_custom_frame, devmouse_attr_matrix_effect_custom);
		colorize(deviceFireflies, 1, 15, offset, devfirefly_attr_matrix_custom_frame, devfirefly_attr_matrix_effect_custom);
		colorize(deviceMugs, 1, 15, offset, devmug_attr_matrix_custom_frame, devmug_attr_matrix_effect_custom);
		colorize(deviceCores, 1, 25, offset, devcore_attr_matrix_custom_frame, devcore_attr_matrix_effect_custom);
		//colorize(deviceKrakens, 9, 5, offset, devkraken_attr_matrix_custom_frame, devkraken_attr_matrix_effect_custom);
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
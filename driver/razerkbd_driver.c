/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Should you need to contact me, the author, you can do so by
 * e-mail - mail your message to Terry Cain <terry@terrys-home.co.uk>
 */


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/dmi.h>

#include "razerkbd_driver.h"
#include "razercommon.h"
#include "razerchromacommon.h"

/*
 * Version Information
 */
#define DRIVER_VERSION "1.1"
#define DRIVER_AUTHOR "Terry Cain <terry@terrys-home.co.uk>"
#define DRIVER_DESC "Razer Keyboard Device Driver"
#define DRIVER_LICENSE "GPL v2"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

// M1-M5 is F13-F17
#define RAZER_MACRO_KEY 188 // 188 = KEY_F18
#define RAZER_GAME_KEY 189 // 189 = KEY_F19
#define RAZER_BRIGHTNESS_DOWN 190 // 190 = KEY_F20
// F21 is used for touchpad disable, F22,F23 is touchpad enable
#define RAZER_BRIGHTNESS_UP 194 // 194 = KEY_F24
#define RAZER_FN 195

#define KEY_FLAG_BLOCK 0b00000001

/**
 * List of keys to swap
 */
static const struct razer_key_translation chroma_keys[] = {
    { KEY_F1,    KEY_MUTE },
    { KEY_F2,    KEY_VOLUMEDOWN },
    { KEY_F3,    KEY_VOLUMEUP },
    
    { KEY_F5,    KEY_PREVIOUSSONG },
    { KEY_F6,    KEY_PLAYPAUSE },
    { KEY_F7,    KEY_NEXTSONG },
    
    { KEY_F9,   RAZER_MACRO_KEY },
    { KEY_F10,  RAZER_GAME_KEY },
    { KEY_F11,  RAZER_BRIGHTNESS_DOWN },
    { KEY_F12,  RAZER_BRIGHTNESS_UP },
    
    { KEY_PAUSE, KEY_SLEEP },
    
    // Custom bind
    { KEY_KPENTER, KEY_CALC },
    { }
};

/**
 * Essentially search through the struct array above.
 */ 
static const struct razer_key_translation *find_translation(const struct razer_key_translation *key_table, u16 from) {
    const struct razer_key_translation *result;
    
    for (result = key_table; result->from; result++) {
        if (result->from == from) {
            return result;
        }
    }
    
    return NULL;
}

static bool is_blade_laptop(struct usb_device *usb_dev) {
    switch (usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_QHD:
            return true;
	}
	return false;
}

/**
 * Send report to the keyboard
 */
int razer_get_report(struct usb_device *usb_dev, struct razer_report *request_report, struct razer_report *response_report) {
    return razer_get_usb_response(usb_dev, 0x02, request_report, 0x02, response_report, RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US, RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US);
}

/**
 * Function to send to device, get response, and actually check the response
 */
struct razer_report razer_send_payload(struct usb_device *usb_dev, struct razer_report *request_report)
{
	int retval = -1;
    struct razer_report response_report;
    
    request_report->crc = razer_calculate_crc(request_report);

    retval = razer_get_report(usb_dev, request_report, &response_report);

    if(retval == 0)
    {
		// Check the packet number, class and command are the same
		if(response_report.remaining_packets != request_report->remaining_packets ||
		   response_report.command_class != request_report->command_class ||
		   response_report.command_id.id != request_report->command_id.id)
		{
			print_erroneous_report(&response_report, "razerkbd", "Response doesnt match request");
//		} else if (response_report.status == RAZER_CMD_BUSY) {
//			print_erroneous_report(&response_report, "razerkbd", "Device is busy");
		} else if (response_report.status == RAZER_CMD_FAILURE) {
			print_erroneous_report(&response_report, "razerkbd", "Command failed");
		} else if (response_report.status == RAZER_CMD_NOT_SUPPORTED) {
			print_erroneous_report(&response_report, "razerkbd", "Command not supported");
		} else if (response_report.status == RAZER_CMD_TIMEOUT) {
			print_erroneous_report(&response_report, "razerkbd", "Command timed out");
		} 
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }
    
    return response_report;
}

/**
 * Device mode function
 */
void razer_set_device_mode(struct usb_device *usb_dev, unsigned char mode, unsigned char param)
{
	struct razer_report report = razer_chroma_standard_set_device_mode(mode, param);

    if (is_blade_laptop(usb_dev)) {
        return;
    }

	switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			report.transaction_id.id = 0x3F;
			break;
	}
	
	razer_send_payload(usb_dev, &report);
}

/**
 * Write device file "mode_game"
 *
 * When 1 is written (as a character, 0x31) Game mode will be enabled, if 0 is written (0x30)
 * then game mode will be disabled
 *
 * The reason the keyboard appears as 2 keyboard devices is that one of those devices is used by
 * game mode as that keyboard device is missing a super key. A hacky and over-the-top way to disable
 * the super key if you ask me.
 */
static ssize_t razer_attr_write_mode_game(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);    
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, GAME_LED, enabled);

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "game_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_mode_game(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, GAME_LED);
    struct razer_report response;

    response = razer_send_payload(usb_dev, &report);
    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "mode_macro"
 *
 * When 1 is written (as a character, 0x31) Macro mode will be enabled, if 0 is written (0x30)
 * then game mode will be disabled
 */
static ssize_t razer_attr_write_mode_macro(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, MACRO_LED, enabled);

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "mode_macro"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_mode_macro(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, MACRO_LED);
    struct razer_report response;

    response = razer_send_payload(usb_dev, &report);
    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Read device file "version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", VERSION);
}

/**
 * Read device file "device_type"
 *
 * Returns friendly string of device type
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    char *device_type;

    switch (usb_dev->descriptor.idProduct)
    {
		case USB_DEVICE_ID_RAZER_ORBWEAVER:
			device_type = "Razer Orbweaver\n";
            break;
		
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
            device_type = "Razer BlackWidow Classic\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012:
            device_type = "Razer BlackWidow Ultimate 2012\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013:
            device_type = "Razer BlackWidow Ultimate 2013\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016:
            device_type = "Razer BlackWidow Ultimate 2016\n";
            break;
            
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_ULTIMATE:
            device_type = "Razer BlackWidow X Ultimate\n";
            break;

        case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
            device_type = "Razer Blade Stealth\n";
            break;

        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
            device_type = "Razer Blade Stealth (Late 2016)\n";
            break;
        
        case USB_DEVICE_ID_RAZER_BLADE_QHD:
            device_type = "Razer Blade Stealth (QHD)\n";
            break;
        
        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
            device_type = "Razer Blade Pro (Late 2016)\n";
            break;
 
        case USB_DEVICE_ID_RAZER_TARTARUS_CHROMA:
            device_type = "Razer Tartarus Chroma\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA:
            device_type = "Razer BlackWidow Chroma\n";
            break;

        case USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA:
            device_type = "Razer DeathStalker Chroma\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE:
            device_type = "Razer BlackWidow Chroma Tournament Edition\n";
            break;
        
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA:
            device_type = "Razer BlackWidow X Chroma\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA_TE:
            device_type = "Razer BlackWidow X Chroma Tournament Edition\n";
            break;
        
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
            device_type = "Razer Ornata Chroma\n";
            break;
        case USB_DEVICE_ID_RAZER_ANANSI:
            device_type = "Razer Anansi\n";
            break;
        default:
            device_type = "Unknown Device\n";
    }

    return sprintf(buf, device_type);
}

/**
 * Write device file "mode_macro_effect"
 *
 * When 1 is written the LED will blink, 0 will static
 */
static ssize_t razer_attr_write_mode_macro_effect(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report;
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    
    switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			report = razer_chroma_standard_set_led_effect(NOSTORE, MACRO_LED, enabled);
			report.transaction_id.id = 0x3F;
			break;

	    case USB_DEVICE_ID_RAZER_ANANSI:
		    report = razer_chroma_standard_set_led_effect(NOSTORE, MACRO_LED, enabled);
		    razer_send_payload(usb_dev, &report);
		    
		    report = razer_chroma_standard_set_led_blinking(NOSTORE, MACRO_LED);
		    break;
		default:
			report = razer_chroma_standard_set_led_effect(VARSTORE, MACRO_LED, enabled);
			break;
	}
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "macro_mode_effect"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_mode_macro_effect(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report = razer_chroma_standard_get_led_effect(VARSTORE, MACRO_LED);
    struct razer_report response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "mode_pulsate"
 *
 * The brightness oscillates between fully on and fully off generating a pulsing effect
 */
static ssize_t razer_attr_write_mode_pulsate(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_set_led_effect(VARSTORE, BACKLIGHT_LED, 0x02);
    
    switch(usb_dev->descriptor.idProduct)
    {
		case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013:
			report = razer_chroma_standard_set_led_effect(VARSTORE, LOGO_LED, 0x02);
			break;
	}  
    
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "mode_pulsate"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_mode_pulsate(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report = razer_chroma_standard_get_led_effect(VARSTORE, LOGO_LED);
    struct razer_report response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Read device file "profile_led_red"
 * 
 * Actually a Yellow LED
 *
 * Returns a string
 */
static ssize_t razer_attr_read_tartarus_profile_led_red(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, RED_PROFILE_LED);
    struct razer_report response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Read device file "profile_led_green"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_tartarus_profile_led_green(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, GREEN_PROFILE_LED);
    struct razer_report response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Read device file "profile_led_blue"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_tartarus_profile_led_blue(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, BLUE_PROFILE_LED);
    struct razer_report response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "profile_led_red"
 */
static ssize_t razer_attr_write_tartarus_profile_led_red(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, RED_PROFILE_LED, enabled);
    
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "profile_led_green"
 */
static ssize_t razer_attr_write_tartarus_profile_led_green(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, GREEN_PROFILE_LED, enabled);
    
    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "profile_led_blue"
 */
static ssize_t razer_attr_write_tartarus_profile_led_blue(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, BLUE_PROFILE_LED, enabled);
    
    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Read device file "get_serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    char serial_string[23];
    struct razer_report report = razer_chroma_standard_get_serial();
    struct razer_report response_report;

    if (is_blade_laptop(usb_dev))
    {
        strcpy(&serial_string[0], dmi_get_system_info(DMI_PRODUCT_SERIAL));

    } else {
        response_report = razer_send_payload(usb_dev, &report);
        strncpy(&serial_string[0], &response_report.arguments[0], 22);
        serial_string[22] = '\0';
    }

    return sprintf(buf, "%s\n", &serial_string[0]);
}

/**
 * Read device file "get_firmware_version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_firmware_version();
    struct razer_report response_report = razer_send_payload(usb_dev, &report);
    
    return sprintf(buf, "v%d.%d", response_report.arguments[0], response_report.arguments[1]);
}

/**
 * Write device file "mode_none"
 *
 * No keyboard effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_mode_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report;
    
    switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			report = razer_chroma_extended_matrix_effect_none(VARSTORE, BACKLIGHT_LED);
			break;

	    case USB_DEVICE_ID_RAZER_ANANSI:
		    report = razer_chroma_standard_set_led_state(VARSTORE, BACKLIGHT_LED, OFF);
		    break;
		default:
			report = razer_chroma_standard_matrix_effect_none(VARSTORE, BACKLIGHT_LED);
			break;
	}
	razer_send_payload(usb_dev, &report);
    
    return count;
}

/**
 * Write device file "mode_wave"
 *
 * When 1 is written (as a character, 0x31) the wave effect is displayed moving left across the keyboard
 * if 2 is written (0x32) then the wave effect goes right
 * 
 * For the extended its 0x00 and 0x01
 */
static ssize_t razer_attr_write_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report;
    
    switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			report = razer_chroma_extended_matrix_effect_wave(VARSTORE, BACKLIGHT_LED, direction);
			break;


		default:
			report = razer_chroma_standard_matrix_effect_wave(VARSTORE, BACKLIGHT_LED, direction);
			break;
	}
	razer_send_payload(usb_dev, &report);
	
    return count;
}

/**
 * Write device file "mode_spectrum"
 *
 * Specrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_mode_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report;
    
    switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			report = razer_chroma_extended_matrix_effect_spectrum(VARSTORE, BACKLIGHT_LED);
			break;

	    case USB_DEVICE_ID_RAZER_ANANSI:
		    report = razer_chroma_standard_set_led_state(VARSTORE, BACKLIGHT_LED, ON);
		    razer_send_payload(usb_dev, &report);
		    report = razer_chroma_standard_set_led_effect(VARSTORE, BACKLIGHT_LED, LED_SPECTRUM_CYCLING);
		    break;
		default:
			report = razer_chroma_standard_matrix_effect_spectrum(VARSTORE, BACKLIGHT_LED);
			break;
	}
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "mode_reactive"
 *
 * Sets reactive mode when this file is written to. A speed byte and 3 RGB bytes should be written
 */
static ssize_t razer_attr_write_mode_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report;
    
    if(count == 4)
    {
		unsigned char speed = (unsigned char)buf[0];
		
		switch(usb_dev->descriptor.idProduct)
		{
			case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
				report = razer_chroma_extended_matrix_effect_reactive(VARSTORE, BACKLIGHT_LED, speed, (struct razer_rgb*)&buf[1]);
				break;


			default:
				report = razer_chroma_standard_matrix_effect_reactive(VARSTORE, BACKLIGHT_LED, speed, (struct razer_rgb*)&buf[1]);
				break;
		}   
		razer_send_payload(usb_dev, &report);
		
    } else {
		printk(KERN_WARNING "razerkbd: Reactive only accepts Speed, RGB (4byte)");
	}
    return count;
}

/**
 * Write device file "mode_static"
 *
 * Set the keyboard to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_mode_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report;
	
    switch(usb_dev->descriptor.idProduct) {
		case USB_DEVICE_ID_RAZER_ORBWEAVER:
			report = razer_chroma_standard_set_led_effect(VARSTORE, BACKLIGHT_LED, 0x00);
            razer_send_payload(usb_dev, &report);
            break;
		
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013: // Doesn't need any parameters as can only do one type of static
			report = razer_chroma_standard_set_led_effect(VARSTORE, LOGO_LED, 0x00);
            razer_send_payload(usb_dev, &report);
            break;
            
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA:
        case USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA_TE:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_ULTIMATE:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_QHD:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
        case USB_DEVICE_ID_RAZER_TARTARUS_CHROMA:
            if(count == 3)
            {
				report = razer_chroma_standard_matrix_effect_static(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
                razer_send_payload(usb_dev, &report);
            } else {
				printk(KERN_WARNING "razerkbd: Static mode only accepts RGB (3byte)");
			}
            break;
        
        
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			if(count == 3)
            {
				report = razer_chroma_extended_matrix_effect_static(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
				razer_send_payload(usb_dev, &report);
			} else {
				printk(KERN_WARNING "razerkbd: Static mode only accepts RGB (3byte)");
			}
			
			break;
        case USB_DEVICE_ID_RAZER_ANANSI:
            if(count == 3)
            {
	            report = razer_chroma_standard_set_led_state(VARSTORE, BACKLIGHT_LED, ON);
	            razer_send_payload(usb_dev, &report);
	            report = razer_chroma_standard_set_led_effect(VARSTORE, BACKLIGHT_LED, LED_STATIC);
	            razer_send_payload(usb_dev, &report);
                report = razer_chroma_standard_set_led_rgb(VARSTORE, BACKLIGHT_LED, (struct razer_rgb *) &buf[0]);
                razer_send_payload(usb_dev, &report);
            }
            else
                printk(KERN_WARNING "razerkbd: Static mode only accepts RGB (3byte)\n");
            break;
            
        default:
            printk(KERN_WARNING "razerkbd: Cannot set static mode for this device\n");
            break;
        
    }

    return count;
}

/**
 * Write device file "mode_startlight"
 *
 * Starlight keyboard effect is activated whenever this file is written to (for bw2016)
 * 
 * Or if an Ornata
 * 7 bytes, speed, rgb, rgb
 * 4 bytes, speed, rgb
 * 1 byte, speed
 */
static ssize_t razer_attr_write_mode_starlight(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_rgb rgb1 = {.r = 0x00, .g = 0xFF, .b = 0x00};
    struct razer_report report;
    
    switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			if(count == 7) {
				report = razer_chroma_extended_matrix_effect_starlight_dual(VARSTORE, BACKLIGHT_LED, buf[0], (struct razer_rgb*)&buf[1], (struct razer_rgb*)&buf[4]);
				razer_send_payload(usb_dev, &report);
			} else if(count == 4) {
				report = razer_chroma_extended_matrix_effect_starlight_single(VARSTORE, BACKLIGHT_LED, buf[0], (struct razer_rgb*)&buf[1]);
				razer_send_payload(usb_dev, &report);
			} else if(count == 1) {
				report = razer_chroma_extended_matrix_effect_starlight_random(VARSTORE, BACKLIGHT_LED, buf[0]);
				razer_send_payload(usb_dev, &report);
			} else {
				printk(KERN_WARNING "razerkbd: Starlight only accepts Speed (1byte). Speed, RGB (4byte). Speed, RGB, RGB (7byte)");
			}
			break;


		default: // BW2016 can do normal starlight
			report = razer_chroma_standard_matrix_effect_starlight_single(VARSTORE, BACKLIGHT_LED, 0x01, &rgb1);
			razer_send_payload(usb_dev, &report);
			break;
	}
    
    return count;
}

/**
 * Write device file "mode_breath"
 */
static ssize_t razer_attr_write_mode_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report;
    
    switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			switch(count) {
				case 3: // Single colour mode
					report = razer_chroma_extended_matrix_effect_breathing_single(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
					razer_send_payload(usb_dev, &report);
					break;
				
				case 6: // Dual colour mode
					report = razer_chroma_extended_matrix_effect_breathing_dual(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
					razer_send_payload(usb_dev, &report);
					break;

				case 1: // "Random" colour mode
					report = razer_chroma_extended_matrix_effect_breathing_random(VARSTORE, BACKLIGHT_LED);
					razer_send_payload(usb_dev, &report);
					break;
				
				default:
					printk(KERN_WARNING "razerkbd: Breathing only accepts '1' (1byte). RGB (3byte). RGB, RGB (6byte)");
					break;
			}
			break;


		default:
			switch(count) {
				case 3: // Single colour mode
					report = razer_chroma_standard_matrix_effect_breathing_single(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
					razer_send_payload(usb_dev, &report);
					break;
				
				case 6: // Dual colour mode
					report = razer_chroma_standard_matrix_effect_breathing_dual(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
					razer_send_payload(usb_dev, &report);
					break;
				
				default: // "Random" colour mode
					report = razer_chroma_standard_matrix_effect_breathing_random(VARSTORE, BACKLIGHT_LED);
					razer_send_payload(usb_dev, &report);
					break;
				// TODO move default to case 1:. Then default: printk(warning). Also remove pointless buffer
			}
			break;
	}

    return count;
}

/**
 * Write device file "set_logo"
 *
 * Sets the logo lighting state to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_set_logo(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char state = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_effect(VARSTORE, LOGO_LED, state);
    
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "mode_custom"
 *
 * Sets the keyboard to custom mode whenever the file is written to
 */
static ssize_t razer_attr_write_mode_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report;
    
    switch(usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			report = razer_chroma_extended_matrix_effect_custom_frame();
			break;


		default:
			report = razer_chroma_standard_matrix_effect_custom_frame(VARSTORE); // Possibly could use VARSTORE
			break;
	}
    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "set_fn_toggle"
 *
 * Sets the logo lighting state to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_set_fn_toggle(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char state = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_misc_fn_key_toggle(state);
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "test"
 *
 * Does nothing
 */
static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

/**
 * Read device file "test"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", 0);
}

/**
 * Write device file "set_brightness"
 *
 * Sets the brightness to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_set_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report;
    
    switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
			report = razer_chroma_extended_matrix_brightness(VARSTORE, BACKLIGHT_LED, brightness);
			break;


        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013:
			report = razer_chroma_standard_set_led_brightness(VARSTORE, LOGO_LED, brightness);
            break;
        
        default:
            if (is_blade_laptop(usb_dev)) {
                report = razer_chroma_misc_set_blade_brightness(brightness);
            } else {
                report = razer_chroma_standard_set_led_brightness(VARSTORE, BACKLIGHT_LED, brightness);
            }
            break;
    }
    razer_send_payload(usb_dev, &report);
    
    return count;
}

/**
 * Read device file "macro_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_set_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
	unsigned char brightness = 0;
    struct razer_report report;
    struct razer_report response;
    
    // If Ornata, dont know if it supports brightness getting as yet
    if (usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_ORNATA_CHROMA)
    {
		return sprintf(buf, "%d\n", brightness);
	}
    
    
    switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012:
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013:
			report = razer_chroma_standard_get_led_brightness(VARSTORE, LOGO_LED);
            break;
    
        default:
            if (is_blade_laptop(usb_dev)) {
                report = razer_chroma_misc_get_blade_brightness();
            } else {
                report = razer_chroma_standard_get_led_brightness(VARSTORE, BACKLIGHT_LED);
            }
            break;
    }

    response = razer_send_payload(usb_dev, &report);

	// Brightness is stored elsewhere for the stealth cmds
    if (is_blade_laptop(usb_dev)) {
        brightness = response.arguments[1];
    } else {
        brightness = response.arguments[2];
    }
    
    
    return sprintf(buf, "%d\n", brightness);
}

/**
 * Write device file "device_mode"
 */
static ssize_t razer_attr_write_device_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report;

    if (count != 2) {
        printk(KERN_WARNING "razerkbd: Device mode only takes 2 bytes.");
        goto out;
    }

    if (is_blade_laptop(usb_dev)) {
        goto out;
    }

    report = razer_chroma_standard_set_device_mode(buf[0], buf[1]);
	razer_send_payload(usb_dev, &report);

out:
    return count;
}

/**
 * Read device file "device_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct usb_interface *intf = to_usb_interface(dev->parent);
	struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report = razer_chroma_standard_get_device_mode();
	struct razer_report response = razer_send_payload(usb_dev, &report);
	
	return sprintf(buf, "%d:%d\n", response.arguments[0], response.arguments[1]);
}

/**
 * Write device file "matrix_custom_frame"
 * 
 * Format
 * ROW_ID START_COL STOP_COL RGB...
 */
static ssize_t razer_attr_write_matrix_custom_frame(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_interface *intf = to_usb_interface(dev->parent);
	struct usb_device *usb_dev = interface_to_usbdev(intf);
	struct razer_report report;
	size_t offset = 0;
	unsigned char row_id;
	unsigned char start_col;
	unsigned char stop_col;
	unsigned char row_length;
	
	//printk(KERN_ALERT "razerkbd: Total count: %d\n", (unsigned char)count);
	
	while(offset < count)
	{
		if(offset + 3 > count)
		{
			printk(KERN_ALERT "razerkbd: Wrong Amount of data provided: Should be ROW_ID, START_COL, STOP_COL, N_RGB\n");
			break;
		}
		
		row_id = buf[offset++];
		start_col = buf[offset++];
		stop_col = buf[offset++];
		row_length = ((stop_col+1) - start_col) * 3;
		
		// printk(KERN_ALERT "razerkbd: Row ID: %d, Start: %d, Stop: %d, row length: %d\n", row_id, start_col, stop_col, row_length);
		
		if(start_col > stop_col)
		{
			printk(KERN_ALERT "razerkbd: Start column is greater than end column\n");
			break;
		}
		
		if(offset + row_length > count)
		{
			printk(KERN_ALERT "razerkbd: Not enough RGB to fill row\n");
			break;
		}
		
		// Offset now at beginning of RGB data
		switch(usb_dev->descriptor.idProduct)
		{
			case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
				report = razer_chroma_extended_matrix_set_custom_frame(row_id, start_col, stop_col, (unsigned char*)&buf[offset]);
				break;
			
			case USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA:
				report = razer_chroma_misc_one_row_set_custom_frame(start_col, stop_col, (unsigned char*)&buf[offset]);
				break;

			case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_ULTIMATE:
			case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016: 
			case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
			case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
			case USB_DEVICE_ID_RAZER_BLADE_QHD:
			case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
				report.transaction_id.id = 0x80; // Fall into the 2016/blade/blade2016 to set device id
			default:
				report = razer_chroma_standard_matrix_set_custom_frame(row_id, start_col, stop_col, (unsigned char*)&buf[offset]);
				break;
		}
		razer_send_payload(usb_dev, &report);
		
		// *3 as its 3 bytes per col (RGB)
		offset += row_length;
	}
	
	
	return count;
}



static ssize_t razer_attr_write_key_super(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    if (count >= 1) {
		device->block_keys[0] = buf[0];
	}
    
    return count;
}

static ssize_t razer_attr_read_key_super(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct razer_kbd_device *device = dev_get_drvdata(dev);

	buf[0] = device->block_keys[0];
	
	return 1;
}


static ssize_t razer_attr_write_key_alt_tab(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    if (count >= 1) {
		printk(KERN_WARNING "razerkbd: Settings block_keys[1] to %u\n", buf[0]);
		device->block_keys[1] = buf[0];
	}
    
    return count;
}

static ssize_t razer_attr_read_key_alt_tab(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct razer_kbd_device *device = dev_get_drvdata(dev);

	buf[0] = device->block_keys[1];
	
	return 1;
}

static ssize_t razer_attr_write_key_alt_f4(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    if (count >= 1) {
		device->block_keys[2] = buf[0];
	}
    
    return count;
}

static ssize_t razer_attr_read_key_alt_f4(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct razer_kbd_device *device = dev_get_drvdata(dev);

	buf[0] = device->block_keys[2];
	
	return 1;
}

/**
 * Set up the device driver files

 *
 * Read only is 0444
 * Write only is 0220
 * Read and write is 0664
 */
// TODO device_mode endpoint
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




/**
 * Deal with FN toggle
 */
static int razer_event(struct hid_device *hdev, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_kbd_device *asc = hid_get_drvdata(hdev);
    const struct razer_key_translation *translation;
    int do_translate = 0;

    // No translations needed on the Blades
    if (is_blade_laptop(usb_dev))
    {
        return 0;
    }


    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
    {
        // Skip this if its control (mouse) interface
        return 0;
    }
    
    // Block win key
    if(asc->block_keys[0] && (usage->code == KEY_LEFTMETA || usage->code == KEY_RIGHTMETA)) {
		return 1;
	}
	
	// Store Alt state
	if(usage->code == KEY_LEFTALT) {
		asc->left_alt_on = value;
	}
	// Block Alt-Tab
	if(asc->block_keys[1] && asc->left_alt_on && usage->code == KEY_TAB) {
		return 1;
	}
	// Block Alt-F4
	if(asc->block_keys[2] && asc->left_alt_on && usage->code == KEY_F4) {
		return 1;
	}
	
	
    
    translation = find_translation(chroma_keys, usage->code);
    
    if(translation)
    {
		if(test_bit(usage->code, asc->pressed_fn))
		{
			do_translate = 1;
		} else {
			do_translate = asc->fn_on;
		}
		
		if(do_translate)
		{
			if(value)
			{
				set_bit(usage->code, asc->pressed_fn);
			} else {
				clear_bit(usage->code, asc->pressed_fn);
			}
			
			input_event(field->hidinput->input, usage->type, translation->to, value);
			return 1;
		}
	}
	
	
	return 0;
}

/**
 * Raw event function
 * 
 * Bastard function. Could most probably be done a load better.
 * Basically it shifts all of the key's in the 04... event to the right 1, and then sets the first 2 bytes to 0x0100. This then allows the keys to be processed with the above normal event function
 * Converts M1-M5 into F13-F17. It also blanks out FN keypresses so it acts more like the modifier it should be.
 * 
 * 04012000000000000000 FN is pressed, M1 pressed
 * 04010000000000000000 M1 is released
 * goes to
 * 01000068000000000000 FN is pressed (blanked), M1 pressed (converted to F13)
 * 01000000000000000000 M1 is released
 * 
 * HID Usage Table http://www.freebsddiary.org/APC/usb_hid_usages.php
 */
static int razer_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct razer_kbd_device *asc = hid_get_drvdata(hdev);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    // No translations needed on the Pro
    if (is_blade_laptop(usb_dev))
    {
        return 0;
    }

    // The event were looking for is 16 bytes long and starts with 0x04
    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD && size == 16 && data[0] == 0x04)
    {
        // Convert 04... to 0100...
        int index = size-1; // This way we start at 2nd last value, does subtract 1 from the 15key rollover though (not an issue cmon)
        u8 cur_value = 0x00;
        int found_fn = 0x00;
        
        while(--index > 0)
        {
			cur_value = data[index];
			if(cur_value == 0x00) { // Skip 0x00
				continue;
			}
			
			switch(cur_value) {
				case 0x01: // FN
					//cur_value = 0x73; // F24
					cur_value = 0x00;
					found_fn = 0x01;
					break;
				case 0x20: // M1
					cur_value = 0x68; // F13
					break;
				case 0x21: // M2
					cur_value = 0x69; // F14
					break;
				case 0x22: // M3
					cur_value = 0x6A; // F15
					break;
				case 0x23: // M4
					cur_value = 0x6B; // F16
					break;
				case 0x24: // M5
					cur_value = 0x6C; // F17
					break;
			}
			
			data[index+1] = cur_value;
		}
		
		asc->fn_on = !!found_fn;
		
		data[0] = 0x01;
		data[1] = 0x00;
		
		// Some reason just by editing data, it generates a normal event above. (Could quite possibly work like that, no clue)
		//hid_report_raw_event(hdev, HID_INPUT_REPORT, data, size, 0);
		return 1;
	}
	
	return 0;
}

/**
 * Probe method is ran whenever a device is binded to the driver
 */
static int razer_kbd_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_kbd_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_kbd_device), GFP_KERNEL);
    if(dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        retval = -ENOMEM;
        goto exit;
    }
    
    // Other interfaces are actual key-emitting devices
    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
    {
        // If the currently bound device is the control (mouse) interface
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);                      // Get the firmware version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);                         // Get serial nubmer
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);                     // Gets and sets the brightness
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);                                  // Test mode
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_mode);                           // Get device mode
        
        switch(usb_dev->descriptor.idProduct) {
            
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012:
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_pulsate);         // Pulsate effect, like breathing
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
                break;
            
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016:
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_ULTIMATE:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
                break;
            
            
            case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
            case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_fn_toggle);                     // Sets wether FN is requires for F-Keys
                break;

            case USB_DEVICE_ID_RAZER_BLADE_QHD:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_fn_toggle);                     // Sets wether FN is requires for F-Keys
                break;

            case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
                break;

            case USB_DEVICE_ID_RAZER_TARTARUS_CHROMA:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_red);               // Profile/Macro LED Red
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_green);             // Profile/Macro LED Green
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_blue);              // Profile/Macro LED Blue
                break;
            
            case USB_DEVICE_ID_RAZER_ORBWEAVER:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
	            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_pulsate);         // Pulsate effect, like breathing
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_red);               // Profile/Macro LED Red
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_green);             // Profile/Macro LED Green
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_blue);              // Profile/Macro LED Blue
                break;
            
            case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
                CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
                break;
			case USB_DEVICE_ID_RAZER_ANANSI:
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_led_state);
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_state);
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_effect);
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
				break;
            
            case USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA:
  				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
				break;
            
			default: // BlackWidow Chroma...
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
				CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
				break;
		}
		
		// Set device to regular mode, not driver mode
		// When the daemon discovers the device it will instruct it to enter driver mode
		razer_set_device_mode(usb_dev, 0x00, 0x00);
	} else if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD) {
		CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_key_super);
		CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_key_alt_tab);
		CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_key_alt_f4);
	}
	
	
	
	hid_set_drvdata(hdev, dev);
	dev_set_drvdata(&hdev->dev, dev);
	
	if(hid_parse(hdev)) {
		hid_err(hdev, "parse failed\n");
		goto exit_free;
	}
	
	if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
		hid_err(hdev, "hw start failed\n");
		goto exit_free;
	}
	
	// Leave autosuspend on for laptops
	if (!is_blade_laptop(usb_dev)) {
		usb_disable_autosuspend(usb_dev);
	}
	
	//razer_activate_macro_keys(usb_dev);
	//msleep(3000);
	return 0;
	exit:
	return retval;
	exit_free:
	kfree(dev);
	return retval;
}

/**
 * Unbind function
 */
static void razer_kbd_disconnect(struct hid_device *hdev)
{
    struct razer_kbd_device *dev;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    dev = hid_get_drvdata(hdev);

    // Other interfaces are actual key-emitting devices    
    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
    {
        // If the currently bound device is the control (mouse) interface
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);                      // Get the firmware version
        device_remove_file(&hdev->dev, &dev_attr_device_serial);                         // Get serial nubmer
        device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);                     // Gets and sets the brightness
        device_remove_file(&hdev->dev, &dev_attr_test);                                  // Test mode
        device_remove_file(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        device_remove_file(&hdev->dev, &dev_attr_device_mode);                           // Get device mode
        
        switch(usb_dev->descriptor.idProduct) {
            
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012:
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013:
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_pulsate);         // Pulsate effect, like breathing
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                device_remove_file(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
                device_remove_file(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
                device_remove_file(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
                break;
            
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016:
            case USB_DEVICE_ID_RAZER_BLACKWIDOW_X_ULTIMATE:
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                device_remove_file(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
                device_remove_file(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
                device_remove_file(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
                break;
            
            
            case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
            case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                device_remove_file(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
                device_remove_file(&hdev->dev, &dev_attr_fn_toggle);                     // Sets wether FN is requires for F-Keys
                break;

            case USB_DEVICE_ID_RAZER_BLADE_QHD:
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                device_remove_file(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
                device_remove_file(&hdev->dev, &dev_attr_fn_toggle);                     // Sets wether FN is requires for F-Keys
                break;

            case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
                device_remove_file(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
                break;

            case USB_DEVICE_ID_RAZER_TARTARUS_CHROMA:
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                device_remove_file(&hdev->dev, &dev_attr_profile_led_red);               // Profile/Macro LED Red
                device_remove_file(&hdev->dev, &dev_attr_profile_led_green);             // Profile/Macro LED Green
                device_remove_file(&hdev->dev, &dev_attr_profile_led_blue);              // Profile/Macro LED Blue
                break;
            
            case USB_DEVICE_ID_RAZER_ORBWEAVER:
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
	            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_pulsate);         // Pulsate effect, like breathing
                device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
                device_remove_file(&hdev->dev, &dev_attr_profile_led_red);               // Profile/Macro LED Red
                device_remove_file(&hdev->dev, &dev_attr_profile_led_green);             // Profile/Macro LED Green
                device_remove_file(&hdev->dev, &dev_attr_profile_led_blue);              // Profile/Macro LED Blue
                break;
            
            case USB_DEVICE_ID_RAZER_ORNATA_CHROMA:
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
				device_remove_file(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
				device_remove_file(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
				device_remove_file(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
				break;
			
			
			case USB_DEVICE_ID_RAZER_ANANSI:
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
				device_remove_file(&hdev->dev, &dev_attr_game_led_state);
				device_remove_file(&hdev->dev, &dev_attr_macro_led_state);
				device_remove_file(&hdev->dev, &dev_attr_macro_led_effect);
				break;
            
            case USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA:
  				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
				device_remove_file(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
				device_remove_file(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
				device_remove_file(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
				break;
            
			default: // BlackWidow Chroma...
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
				device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
				device_remove_file(&hdev->dev, &dev_attr_game_led_state);                // Enable game mode & LED
				device_remove_file(&hdev->dev, &dev_attr_macro_led_state);               // Enable macro LED
				device_remove_file(&hdev->dev, &dev_attr_macro_led_effect);              // Change macro LED effect (static, flashing)
				break;
		}
	} else if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD) {
		device_remove_file(&hdev->dev, &dev_attr_key_super);
		device_remove_file(&hdev->dev, &dev_attr_key_alt_tab);
		device_remove_file(&hdev->dev, &dev_attr_key_alt_f4);
	}
	
	hid_hw_stop(hdev);
	kfree(dev);
	dev_info(&intf->dev, "Razer Device disconnected\n");
}

/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_ORBWEAVER) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_X_ULTIMATE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_QHD) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_TARTARUS_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DEATHSTALKER_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_X_CHROMA_TE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_ORNATA_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_ANANSI) },
    { }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

/**
 * Describes the contents of the driver
 */
static struct hid_driver razer_kbd_driver = {
		.name = "razerkbd",
		.id_table = razer_devices,
		.probe = razer_kbd_probe,
		.remove = razer_kbd_disconnect,
		
		.event = razer_event,
		.raw_event = razer_raw_event,
};

module_hid_driver(razer_kbd_driver);



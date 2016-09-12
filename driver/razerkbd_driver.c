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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <pez2001@voyagerproject.de>, or by paper mail:
 * Tim Theede, Am See 22, 24790 Schuelldorf, Germany
 * Terry Cain <terrys-home.co.uk>
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

#define RAZER_FN_KEY 194 // 194 = KEY_F24
#define RAZER_MACRO_KEY 188 // 188 = KEY_F18
#define RAZER_GAME_KEY 189 // 189 = KEY_F19

#define KEY_FLAG_BLOCK 0b00000001

static const struct razer_key_translation chroma_keys[] = {
    { KEY_F1,    KEY_MUTE },
    { KEY_F2,    KEY_VOLUMEDOWN },
    { KEY_F3,    KEY_VOLUMEUP },
    
    { KEY_F5,    KEY_PREVIOUSSONG },
    { KEY_F6,   KEY_PLAYPAUSE },
    { KEY_F7,   KEY_NEXTSONG },
    
    { KEY_F9,   RAZER_MACRO_KEY },
    { KEY_F10,  RAZER_GAME_KEY },
    { KEY_F11,  0, KEY_FLAG_BLOCK },
    { KEY_F12,  0, KEY_FLAG_BLOCK },
    
    { KEY_PAUSE, KEY_SLEEP },
    
    // Custom bind
    { KEY_KPENTER, KEY_CALC },
    { }
};

static const struct razer_key_translation *find_translation(const struct razer_key_translation *key_table, u16 from) {
    const struct razer_key_translation *result;
    
    for (result = key_table; result->from; result++) {
        if (result->from == from) {
            return result;
        }
    }
    
    return NULL;
}


/*

    TODO:

        restore store rgb profile (helpful for event-animations etc)
        #coloritup update

    future todos:

        read keystroke stats etc.

*/

/**
 * Send report to the keyboard
 */
int razer_set_report(struct usb_device *usb_dev,void const *data) {
    return razer_send_control_msg(usb_dev, data, 0x02, RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US, RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US);
}

int razer_get_report(struct usb_device *usb_dev, struct razer_report *request_report, struct razer_report *response_report) {
    return razer_get_usb_response(usb_dev, 0x02, request_report, 0x02, response_report, RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US, RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US);
}





/**
 * Get firmware version
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013?
 *   Razer BlackWidow Ultimate 2016?
 */
int razer_get_firmware_version(struct usb_device *usb_dev, unsigned char* fw_string)
{
    int retval = -1;
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x00, 0x81, 0x02);
    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x00 && response_report.command_id.id == 0x81)
        {
            sprintf(fw_string, "v%d.%d", response_report.arguments[0], response_report.arguments[1]);
            retval = response_report.arguments[2];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return retval;
}

/**
 * Set pulsate effect on the keyboard
 *
 *
 * Supported by:
 *   Razer BlackWidow Ultimate 2013
 *
 * Still not working
 */
int razer_set_pulsate_mode(struct usb_device *usb_dev)
{
    int retval = 0;
    struct razer_report report = get_razer_report(0x03, 0x02, 0x03);

    report.arguments[0] = 0x01; // LED Class, maybe profile?
    report.arguments[1] = 0x04; // LED ID, should be Logo?
    report.arguments[2] = 0x02; // Effect ID
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);

    return retval;
}

/**
 * Get macro on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Ultimate 2013?
 */
int razer_get_pulsate_mode(struct usb_device *usb_dev)
{
    int retval = -1;
    // Class LED Lighting, Command Set state, 3 Bytes of parameters
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x03, 0x82, 0x03);
    request_report.arguments[0] = 0x01; // LED Class, profile 1?
    request_report.arguments[1] = 0x04; // LED ID, Logo LED
    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x03 && response_report.command_id.id == 0x82)
        {
            retval = response_report.arguments[2];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return retval;
}

/**
 * Get the devices serial number
 *
 * Makes a request like normal, this must change a variable in the mouse as then we
 * tell it give us data (same request for get_battery in the mouse driver) and it 
 * gives us a report.
 *
 * Supported Devices:
 *   Razer Chroma
 *   Razer BlackWidow Ultimate 2013*
 * 
 * *Untested but should work
 */
void razer_get_serial(struct usb_device *usb_dev, unsigned char* serial_string)
{
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x00, 0x82, 0x16);
    int retval;
    int i;
    if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH)
    {
        // Stealth does not have serial via USB, so get it from DMI table
        strcpy(serial_string, dmi_get_system_info(DMI_PRODUCT_SERIAL));
    } else
    {
        // Get serial
        request_report.crc = razer_calculate_crc(&request_report);

        retval = razer_get_report(usb_dev, &request_report, &response_report);

        if(retval == 0)
        {
            if(response_report.status == 0x02 && response_report.command_class == 0x00 && response_report.command_id.id == 0x82)
            {
                unsigned char* pointer = &response_report.arguments[0];
                for(i = 0; i < 20; ++i)
                {
                    serial_string[i] = *pointer;
                    ++pointer;
                }
            } else
            {
                print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
            }
        } else
        {
          print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
        }
    }
}

/**
 * Set game mode on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 */
int razer_set_game_mode(struct usb_device *usb_dev, unsigned char enable)
{
    int retval = 0;
    if(enable > 1)
    {
        printk(KERN_WARNING "razerkbd: Cannot set game mode to %d. Only 1 or 0 allowed.", enable);
    } else
    {
        // Class LED Lighting, Command Set state, 3 Bytes of parameters
        struct razer_report report = get_razer_report(0x03, 0x00, 0x03);
        report.arguments[0] = 0x01; // LED Class, profile 1?
        report.arguments[1] = 0x08; // LED ID, Game mode LED
        report.arguments[2] = enable; // Enable 1/0
        report.crc = razer_calculate_crc(&report);
        retval = razer_set_report(usb_dev, &report);
    }
    return retval;
}

/**
 * Get game mode on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 */
int razer_get_game_mode(struct usb_device *usb_dev)
{
    int retval = -1;
    // Class LED Lighting, Command Set state, 3 Bytes of parameters
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x03, 0x80, 0x03);
    request_report.arguments[0] = 0x01; // LED Class, profile 1?
    request_report.arguments[1] = 0x08; // LED ID, Game mode LED
    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x03 && response_report.command_id.id == 0x80)
        {
            retval = response_report.arguments[2];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return retval;
}

/**
 * Set macro led on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 */
int razer_set_macro_led_mode(struct usb_device *usb_dev, unsigned char enable)
{
    int retval = 0;
    if(enable > 2)
    {
        printk(KERN_WARNING "razerkbd: Cannot set game mode to %d. Only 1 or 0 allowed.", enable);
    } else
    {
        // Class LED Lighting, Command Set state, 3 Bytes of parameters
        struct razer_report report = get_razer_report(0x03, 0x00, 0x03);
        report.arguments[0] = 0x01; // LED Class, profile 1?
        report.arguments[1] = 0x07; // LED ID, Macro LED
        report.arguments[2] = enable; // Enable 1/0
        report.crc = razer_calculate_crc(&report);
        retval = razer_set_report(usb_dev, &report);
    }
    return retval;
}

/**
 * Set macro led effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 */
int razer_set_macro_led_effect(struct usb_device *usb_dev, unsigned char enable)
{
    int retval = 0;
    if(enable > 1)
    {
        printk(KERN_WARNING "razerkbd: Cannot set game mode to %d. Only 1 or 0 allowed.", enable);
    } else
    {
        // Class LED Lighting, Command Set state, 3 Bytes of parameters
        struct razer_report report = get_razer_report(0x03, 0x02, 0x03);
        report.arguments[0] = 0x01; // LED Class, profile 1?
        report.arguments[1] = 0x07; // LED ID, Macro LED
        report.arguments[2] = enable; // Enable 1/0
        report.crc = razer_calculate_crc(&report);
        retval = razer_set_report(usb_dev, &report);
    }
    return retval;
}
/**
 * Get macro on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013?
 *   Razer BlackWidow Ultimate 2016?
 */
int razer_get_macro_led_mode(struct usb_device *usb_dev)
{
    int retval = -1;
    // Class LED Lighting, Command Set state, 3 Bytes of parameters
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x03, 0x80, 0x03);
    request_report.arguments[0] = 0x01; // LED Class, profile 1?
    request_report.arguments[1] = 0x07; // LED ID, Game mode LED
    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x03 && response_report.command_id.id == 0x80)
        {
            retval = response_report.arguments[2];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return retval;
}

/**
 * Get status/macroprofile LED
 *
 * Supported by:
 *   Razer Tartarus Chroma
 */
int razer_get_tartarus_status_led(struct usb_device *usb_dev, unsigned char led) {
    int retval = -1;
    // Class LED Lighting, Command Set state, 3 Bytes of parameters
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x03, 0x80, 0x03);
    request_report.arguments[0] = 0x01;
    request_report.arguments[1] = led; // LED ID: Red (0x0C), Green (0x0D) or Blue (0x0D) keymap LED
    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x03 && response_report.command_id.id == 0x80)
        {
            retval = response_report.arguments[2];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return retval;
}

/**
 * Set status/macroprofile LED
 *
 * Supported by:
 *   Razer Tartarus Chroma
 */
int razer_set_tartarus_status_led(struct usb_device *usb_dev, unsigned char led, unsigned char enable)
{
    int retval = 0;
    if(enable > 1)
    {
        printk(KERN_WARNING "razerkbd: Cannot set status/macroprofile mode to %d. Only 1 or 0 allowed.", enable);
    } else
    {
        // Class LED Lighting, Command Set state, 3 Bytes of parameters
        struct razer_report report = get_razer_report(0x03, 0x00, 0x03);
        report.arguments[0] = 0x01;
        report.arguments[1] = led; // LED ID: Red (0x0C), Green (0x0D) or Blue (0x0D) keymap LED
        report.arguments[2] = enable; // Enable 1/0
        report.crc = razer_calculate_crc(&report);
        retval = razer_set_report(usb_dev, &report);
    }
    return retval;
}

/**
 * Get macro_effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013?
 *   Razer BlackWidow Ultimate 2016?
 */
int razer_get_macro_led_effect(struct usb_device *usb_dev)
{
    int retval = -1;
    // Class LED Lighting, Command Set state, 3 Bytes of parameters
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x03, 0x82, 0x03);
    request_report.arguments[0] = 0x01; // LED Class, profile 1?
    request_report.arguments[1] = 0x07; // LED ID, Game mode LED
    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x03 && response_report.command_id.id == 0x82)
        {
            retval = response_report.arguments[2];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return retval;
}

/**
 * Set the wave effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Ultimate 2016
 *
 *   TODO improve as can do more
 */
int razer_set_starlight_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x09);
    report.arguments[0] = 0x19; // Effect ID
    report.arguments[1] = 0x01; // Type one color
    report.arguments[2] = 0x01; // Speed

    report.arguments[3] = 0x00; // Red 1
    report.arguments[4] = 0xFF; // Green 1
    report.arguments[5] = 0x00; // Blue 1

    report.arguments[6] = 0x00; // Red 2
    report.arguments[7] = 0x00; // Green 2
    report.arguments[8] = 0x00; // Blue 2

    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set the wave effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2016
 *   Razer Blade Stealth
 */
int razer_set_wave_mode(struct usb_device *usb_dev, unsigned char direction)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x02);
    report.arguments[0] = 0x01; // Effect ID
    report.arguments[1] = direction; // Direction
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set no effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2016
 *   Razer Blade Stealth
 */
int razer_set_none_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x00; // Effect ID
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set reactive effect on the keyboard
 *
 * The speed must be within 01-04
 *
 * 1 Short, 2 Medium, 3 Long
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2016
 *   Razer Blade Stealth
 */
int razer_set_reactive_mode(struct usb_device *usb_dev, struct razer_rgb *color, unsigned char speed)
{
    int retval = 0;
    if(speed > 0 && speed < 5)
    {
        struct razer_report report = get_razer_report(0x03, 0x0A, 0x05);
        report.arguments[0] = 0x02; // Effect ID
        report.arguments[1] = speed; // Time
        report.arguments[2] = color->r; /*rgb color definition*/
        report.arguments[3] = color->g;
        report.arguments[4] = color->b;
        report.crc = razer_calculate_crc(&report);
        retval = razer_set_report(usb_dev, &report);
    } else
    {
        printk(KERN_WARNING "razerkbd: Reactive mode, Speed must be within 1-4. Got: %d", speed);
    }
    return retval;
}

/**
 * Set breath effect on the keyboard
 *
 * Breathing types
 * 1: Only 1 Colour
 * 2: 2 Colours
 * 3: Random
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2016
 *   Razer Blade Stealth
 */
int razer_set_breath_mode(struct usb_device *usb_dev, unsigned char breathing_type, struct razer_rgb *color1, struct razer_rgb *color2)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x08);
    report.arguments[0] = 0x03; // Effect ID

    report.arguments[1] = breathing_type;

    if(breathing_type == 1 || breathing_type == 2)
    {
        // Colour 1
        report.arguments[2] = color1->r;
        report.arguments[3] = color1->g;
        report.arguments[4] = color1->b;
    }

    if(breathing_type == 2)
    {
        // Colour 2
        report.arguments[5] = color2->r;
        report.arguments[6] = color2->g;
        report.arguments[7] = color2->b;
    }

    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set spectrum effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer Blade Stealth
 */
int razer_set_spectrum_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x04; // Effect ID
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set custom effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2016 (Not working :( )
 */
int razer_set_custom_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x02);
    report.arguments[0] = 0x05; // Effect ID
    report.arguments[1] = 0x01; // Data frame ID

    if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016 ||
       usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH)
    {
        report.arguments[1] = 0x00; /*Data frame ID ?*/
    }
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set static effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2016
 *   Razer Blade Stealth
 */
int razer_set_static_mode(struct usb_device *usb_dev, struct razer_rgb *color)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x04);
    report.arguments[0] = 0x06; // Effect ID
    report.arguments[1] = color->r; /*rgb color definition*/
    report.arguments[2] = color->g;
    report.arguments[3] = color->b;
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set static effect on the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Ultimate 2013
 */
int razer_set_static_mode_blackwidow_ultimate(struct usb_device *usb_dev)
{
    int retval = 0;
    struct razer_report report = get_razer_report(0x03, 0x02, 0x03);
    report.arguments[0] = 0x01; // LED Class, profile?
    report.arguments[1] = 0x04; // LED ID, Logo?
    report.arguments[2] = 0x00; // Effect Static
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Clear row on the keyboard
 *
 * Clears a row's colour on the keyboard. Rows range from 0-5, 0 being the top row with escape
 * and 5 being the last row with the spacebar
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 */
int razer_temp_clear_row(struct usb_device *usb_dev, unsigned char row_index)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x02);
    report.arguments[0] = 0x08; // Clear Row Effect
    report.arguments[1] = row_index; // Row ID
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set row colour on the keyboard
 *
 * This sets the colour of a row on the keyboard. Takes in an array of RGB bytes.
 * The mappings below are correct for the BlackWidow Chroma. The BlackWidow Ultimate 2016
 * contains LEDs under the spacebar and the FN key so there will be changes once I get the
 * hardware.
 *
 * Row 0:
 *  0      Unused
 *  1      ESC
 *  2      Unused
 *  3-14   F1-F12
 *  15-17  PrtScr, ScrLk, Pause
 *  18-19  Unused
 *  20     Razer Logo
 *  21     Unused
 *
 * Row 1:
 *  0-21   M1 -> NP Minus
 *
 * Row 2:
 *  0-13   M2 -> Right Square Bracket ]
 *  14 Unused
 *  15-21 Delete -> NP Plus
 *
 * Row 3:
 *  0-14   M3 -> Return
 *  15-17  Unused
 *  18-20  NP4 -> NP6
 *
 * Row 4:
 *  0-12   M4 -> Forward Slash /
 *  13     Unused
 *  14     Right Shift
 *  15     Unused
 *  16     Up Arrow Key
 *  17     Unused
 *  18-21  NP1 -> NP Enter
 *
 * Row 5:
 *  0-3    M5 -> Alt
 *  4-10   Unused
 *  11     Alt GR
 *  12     Unused
 *  13-17  Context Menu Key -> Right Arrow Key
 *  18     Unused
 *  19-20  NP0 -> NP.
 *  21     Unused
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2016 (Not working :( )
 */
int razer_set_key_row(struct usb_device *usb_dev, unsigned char row_index, unsigned char *row_cols) //struct razer_row_rgb *row_cols)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0B, 0x46);
    unsigned char row_length = RAZER_BLACKWIDOW_CHROMA_ROW_LEN;

    // Ultimate 2016 and stealth use 0x80
    if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016 ||
       usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH)
    {
        report.transaction_id.id = 0x80;
    }

    // Added this to handle variable row lengths
    if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH)
    {
        row_length = RAZER_STEALTH_ROW_LEN;
    }

    report.data_size = row_length * 3 + 4;

    report.arguments[0] = 0xFF;/* Frame ID */
    report.arguments[1] = row_index; /* Row */
    report.arguments[2] = 0x00; /* Start Index 0-21*/
    report.arguments[3] = row_length - 1; /* End Index 0-21 (calculated to end of row)*/
    memcpy(&report.arguments[4], row_cols, (report.arguments[3]+1)*3);

    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Reset the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 */
int razer_reset(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x00, 0x05);
    report.arguments[0] = 0x01; // LED Class, profile?
    report.arguments[1] = 0x08; // LED ID Game mode
    report.arguments[2] = 0x00; // Off
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set the keyboard brightness
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 *   Razer Stealth?????
 */
int razer_set_brightness(struct usb_device *usb_dev, unsigned char brightness)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x03, 0x03);
    report.arguments[0] = 0x01; // LED Class, profile?

    if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH) {
        report.data_size = 0x02;
        report.command_class = 0x0E;
        report.command_id.id = 0x04;
        report.arguments[1] = brightness;

    } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012 ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013)
    {
        report.arguments[1] = 0x04;/* LED ID, Logo */
        report.arguments[2] = brightness;
    } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_X ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016 ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_TARTARUS_CHROMA)
    {
        report.arguments[1] = 0x05;/* Backlight LED */
        report.arguments[2] = brightness;
    } else {
        printk(KERN_WARNING "razerkbd: Unknown product ID '%d'", usb_dev->descriptor.idProduct);
    }

    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Get brightness of the keyboard
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 */
int razer_get_brightness(struct usb_device *usb_dev)
{
    int retval = -1;
    // Class LED Lighting, Command Set state, 3 Bytes of parameters
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x03, 0x83, 0x03);
    request_report.arguments[0] = 0x01; // LED Class, profile 1?

    if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH) {
        request_report.data_size = 0x02;
        request_report.command_class = 0x0E;
        request_report.command_id.id = 0x84;

    } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012 ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013)
    {
        request_report.arguments[1] = 0x04;/* LED ID, Logo */
    } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_X ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016 ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_TARTARUS_CHROMA)
    {
        request_report.arguments[1] = 0x05;/* Backlight LED */
    } else {
        printk(KERN_WARNING "razerkbd: Unknown product ID '%d'", usb_dev->descriptor.idProduct);
    }

    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x03 && response_report.command_id.id == 0x83) // For others
        {
            retval = response_report.arguments[2];
        } else if(response_report.status == 0x02 && response_report.command_class == 0x0E && response_report.command_id.id == 0x84) // For razer stealth
        {
            retval = response_report.arguments[1];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
            retval = -1;
        }
    } else
    {
        print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
        retval = -1;
    }

    return retval;
}

/**
 * Toggle FN key
 * 
 * If 0 should mean that the F-keys work as normal F-keys
 * If 1 should mean that the F-keys act as if the FN key is held
 *
 * Supported by:
 *   Razer Blade Stealth????
 */
int razer_set_fn_toggle(struct usb_device *usb_dev, unsigned char state)
{
    int retval = 0;
    if (state == 0 || state == 1)
    {
      struct razer_report report = get_razer_report(0x02, 0x06, 0x02);
      report.arguments[0] = 0x00; // ??
      report.arguments[1] = state; // State
      report.crc = razer_calculate_crc(&report);
      retval = razer_set_report(usb_dev, &report);
    } else
    {
        printk(KERN_WARNING "razerkbd: Toggle FN, state must be either 0 or 1. Got: %d", state);
    }
    return retval;
}

/**
 * Set the logo lighting state (on/off only)
 *
 * Supported by:
 *   Razer Blade Stealth
 */
int razer_set_logo(struct usb_device *usb_dev, unsigned char state)
{
    int retval = 0;
    if (state == 0 || state == 1)
    {
        struct razer_report report = get_razer_report(0x03, 0x00, 0x03);
        report.arguments[0] = 0x01; // LED Class, profile?
        report.arguments[1] = 0x04; // LED ID, Logo
        report.arguments[2] = state; // Off
        report.crc = razer_calculate_crc(&report);
        retval = razer_set_report(usb_dev, &report);
    } else
    {
        printk(KERN_WARNING "razerkbd: Logo lighting, state must be either 0 or 1. Got: %d", state);
    }
    return retval;
}

/**
 * Enable keyboard macro keys
 *
 * The keycodes for the macro keys are 191-195 for M1-M5.
 * Been tested on the Chroma and the Ultimate
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013
 *   Razer BlackWidow Ultimate 2016
 */
int razer_activate_macro_keys(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x00, 0x04, 0x02); // Device Mode
    report.arguments[0] = 0x02; // Unknown
    report.arguments[1] = 0x04; // Parm 0x00
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

//TESTS
/**
 * Not sure what this does.
 */
int razer_test(struct usb_device *usb_dev, int temp)
{   //Status  ID  Packet Num  Size  Class Command  Params
    
    int retval;
    struct razer_report report = get_razer_report(0x00, 0x04, 0x02); // Device Mode
    report.arguments[0] = 0x03; // Unknown
    report.arguments[1] = 0x00; // Parm 0x00
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    
    printk(KERN_WARNING "razerkbd: Test mode");
    
    return retval;
}

/**
 * Get raw event from the keyboard
 *
 * Useful if the keyboard's 2 keyboard devices are binded then keypress's can be
 * monitored and used.
 */
 /*
static int razer_raw_event(struct hid_device *hdev,
    struct hid_report *report, u8 *data, int size)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    //struct razer_kbd_device *widow = hid_get_drvdata(hdev);

    if (intf->cur_altsetting->desc.bInterfaceProtocol != USB_INTERFACE_PROTOCOL_MOUSE)
        return 0;

    return 0;
} */

/**
 * Read device file "profile_led_red"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_tartarus_profile_led_red(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    int brightness = razer_get_tartarus_status_led(usb_dev, 0x0E);
    return sprintf(buf, "%d\n", brightness);
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

    int brightness = razer_get_tartarus_status_led(usb_dev, 0x0C);
    return sprintf(buf, "%d\n", brightness);
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

    int brightness = razer_get_tartarus_status_led(usb_dev, 0x0D);
    return sprintf(buf, "%d\n", brightness);
}


/**
 * Write device file "profile_led_red"
 */
static ssize_t razer_attr_write_tartarus_profile_led_red(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    if(temp == 1) {
        razer_set_tartarus_status_led(usb_dev, 0x0E, 0x01);
    } else {
        razer_set_tartarus_status_led(usb_dev, 0x0E, 0x00);
    } 

    return count;
}
/**
 * Write device file "profile_led_green"
 */
static ssize_t razer_attr_write_tartarus_profile_led_green(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    if(temp == 1) {
        razer_set_tartarus_status_led(usb_dev, 0x0C, 0x01);
    } else {
        razer_set_tartarus_status_led(usb_dev, 0x0C, 0x00);
    } 
    return count;
}
/**
 * Write device file "profile_led_blue"
 */
static ssize_t razer_attr_write_tartarus_profile_led_blue(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    if(temp == 1) {
        razer_set_tartarus_status_led(usb_dev, 0x0D, 0x01);
    } else {
        razer_set_tartarus_status_led(usb_dev, 0x0D, 0x00);
    } 
    return count;
}

/**
 * Write device file "test"
 *
 * Does nothing
 */
static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    razer_test(usb_dev, temp);
    return count;
}

/**
 * Read device file "test"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    //struct usb_interface *intf = to_usb_interface(dev->parent);
    //struct usb_device *usb_dev = interface_to_usbdev(intf);

    //int test = razer_test(usb_dev);
    return sprintf(buf, "%d\n", 0);
}

/**
 * Write device file "reset"
 *
 * Resets the keyboard whenever anything is written
 */
static ssize_t razer_attr_write_reset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_reset(usb_dev);
    return count;
}

/**
 * Write device file "macro_keys"
 *
 * Enables the macro keys whenever the file is written to
 */
static ssize_t razer_attr_write_macro_keys(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_activate_macro_keys(usb_dev);
    return count;
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
    int temp = simple_strtoul(buf, NULL, 10);
    razer_set_game_mode(usb_dev, temp);
    return count;
}

/**
 * Write device file "mode_macro"
 *
 * When 1 is written (as a character, 0x31) Game mode will be enabled, if 0 is written (0x30)
 * then game mode will be disabled
 *
 * The reason the keyboard appears as 2 keyboard devices is that one of those devices is used by
 * game mode as that keyboard device is missing a super key. A hacky and over-the-top way to disable
 * the super key if you ask me.
 */
static ssize_t razer_attr_write_mode_macro(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    razer_set_macro_led_mode(usb_dev, temp);
    return count;
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
    int temp = simple_strtoul(buf, NULL, 10);
    razer_set_macro_led_effect(usb_dev, temp);
    return count;
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
    razer_set_pulsate_mode(usb_dev);
    return count;
}

/**
 * Write device file "mode_wave"
 *
 * When 1 is written (as a character, 0x31) the wave effect is displayed moving left across the keyboard
 * if 2 is written (0x32) then the wave effect goes right
 */
static ssize_t razer_attr_write_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    razer_set_wave_mode(usb_dev, temp);
    return count;
}

/**
 * Write device file "mode_spectrum"
 *
 * Specrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_mode_spectrum(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_spectrum_mode(usb_dev);
    return count;
}

/**
 * Write device file "mode_startlight"
 *
 * No keyboard effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_mode_starlight(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_starlight_mode(usb_dev);
    return count;
}

/**
 * Write device file "mode_none"
 *
 * No keyboard effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_mode_none(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_none_mode(usb_dev);
    return count;
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
    int temp = simple_strtoul(buf, NULL, 10);
    razer_set_brightness(usb_dev, (unsigned char)temp);
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
    int state = simple_strtoul(buf, NULL, 10);
    razer_set_fn_toggle(usb_dev, (unsigned char)state);
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
    int state = simple_strtoul(buf, NULL, 10);
    razer_set_logo(usb_dev, (unsigned char)state);
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
    if(count == 4)
    {
        unsigned char speed = (unsigned char)buf[0];
        razer_set_reactive_mode(usb_dev, (struct razer_rgb*)&buf[1], speed);
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

    const char *alt_buf[6] = { 0 };

    if(count == 3)
    {
        // Single colour mode
        razer_set_breath_mode(usb_dev, 0x01, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&alt_buf[3]);
    } else if(count == 6)
    {
        // Dual colour mode
        razer_set_breath_mode(usb_dev, 0x02, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
    } else
    {
        // "Random" colour mode
        razer_set_breath_mode(usb_dev, 0x03, (struct razer_rgb*)&alt_buf[0], (struct razer_rgb*)&alt_buf[3]);
    }
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
    razer_reset(usb_dev);
    razer_set_custom_mode(usb_dev);
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

    if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL ||
       usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012 ||
       usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013)
    {
        // Set BlackWidow Ultimate to static colour
        razer_set_static_mode_blackwidow_ultimate(usb_dev);

    } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_X ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016 ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH ||
              usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_TARTARUS_CHROMA)
    {
        // Set BlackWidow Chroma to static colour
        if(count == 3)
        {
            razer_set_static_mode(usb_dev, (struct razer_rgb*)&buf[0]);
        }

    } else
    {
        printk(KERN_WARNING "razerkbd: Cannot set static mode for this device");
    }
    return count;
}

/**
 * Write device file "temp_clear_row"
 *
 * Clears a row when an ASCII number is written
 */
static ssize_t razer_attr_write_temp_clear_row(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    razer_temp_clear_row(usb_dev, temp);
    return count;
}

/**
 * Write device file "set_key_row"
 *
 * Writes the colour rows on the keyboard. Takes in all the colours for the keyboard
 */
static ssize_t razer_attr_write_set_key_row(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    size_t offset = 0;
    unsigned char row_index;
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    size_t buf_size = RAZER_BLACKWIDOW_CHROMA_ROW_LEN * 3 + 1;

    while(offset < count) {
        if((count-offset) < buf_size) {
            printk(KERN_ALERT "Wrong Amount of RGB data provided: %d of %d\n",(int)(count-offset), (int)buf_size);
            return -EINVAL;
        }
        row_index = (unsigned char)buf[offset];
        razer_set_key_row(usb_dev, row_index, (unsigned char*)&buf[offset + 1]);
        offset += buf_size;
    }
    return count;
}

/**
 * Read device file "device_type"
 *
 * Returns friendly string of device type
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    char *device_type;

    switch (usb_dev->descriptor.idProduct)
    {
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL:
            device_type = "Razer BlackWidow Original\n";
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

        case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
            device_type = "Razer Blade Stealth\n";
            break;
        
        case USB_DEVICE_ID_RAZER_TARTARUS_CHROMA:
            device_type = "Razer Tartarus Chroma\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA:
            device_type = "Razer BlackWidow Chroma\n";
            break;

        case USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE:
            device_type = "Razer BlackWidow Chroma Tournament Edition\n";
            break;
        
        case USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_X:
            device_type = "Razer BlackWidow X Chroma\n";
            break;

        default:
            device_type = "Unknown Device\n";
    }

    return sprintf(buf, device_type);
}

/**
 * Read device file "get_serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    char serial_string[100] = ""; // Cant be longer than this as report length is 90
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    razer_get_serial(usb_dev, &serial_string[0]);
    return sprintf(buf, "%s\n", &serial_string[0]);
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

    int game_mode = razer_get_game_mode(usb_dev);
    return sprintf(buf, "%d\n", game_mode);
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

    int game_mode = razer_get_pulsate_mode(usb_dev);
    return sprintf(buf, "%d\n", game_mode);
}

/**
 * Read device file "macro_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_mode_macro(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    int macro_mode = razer_get_macro_led_mode(usb_dev);
    return sprintf(buf, "%d\n", macro_mode);
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

    int macro_mode = razer_get_macro_led_effect(usb_dev);
    return sprintf(buf, "%d\n", macro_mode);
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

    int brightness = razer_get_brightness(usb_dev);
    return sprintf(buf, "%d\n", brightness);
}

/**
 * Read device file "get_firmware_version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    char fw_string[100] = ""; // Cant be longer than this as report length is 90
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    razer_get_firmware_version(usb_dev, &fw_string[0]);
    return sprintf(buf, "%s\n", &fw_string[0]);
}



/**
 * Set up the device driver files

 *
 * Read only is 0444
 * Write only is 0220
 * Read and write is 0664
 */
static DEVICE_ATTR(mode_game,         0660, razer_attr_read_mode_game,         razer_attr_write_mode_game);
static DEVICE_ATTR(mode_macro,        0660, razer_attr_read_mode_macro,        razer_attr_write_mode_macro);
static DEVICE_ATTR(set_brightness,    0660, razer_attr_read_set_brightness,    razer_attr_write_set_brightness);
static DEVICE_ATTR(mode_macro_effect, 0660, razer_attr_read_mode_macro_effect, razer_attr_write_mode_macro_effect);
static DEVICE_ATTR(mode_pulsate,      0660, razer_attr_read_mode_pulsate,      razer_attr_write_mode_pulsate);

static DEVICE_ATTR(profile_led_red,   0660, razer_attr_read_tartarus_profile_led_red,   razer_attr_write_tartarus_profile_led_red);
static DEVICE_ATTR(profile_led_green, 0660, razer_attr_read_tartarus_profile_led_green, razer_attr_write_tartarus_profile_led_green);
static DEVICE_ATTR(profile_led_blue,  0660, razer_attr_read_tartarus_profile_led_blue,  razer_attr_write_tartarus_profile_led_blue);


static DEVICE_ATTR(device_type,          0440, razer_attr_read_device_type,          NULL);
static DEVICE_ATTR(get_serial,           0440, razer_attr_read_get_serial,           NULL);
static DEVICE_ATTR(get_firmware_version, 0440, razer_attr_read_get_firmware_version, NULL);

static DEVICE_ATTR(mode_starlight,    0220, NULL, razer_attr_write_mode_starlight);
static DEVICE_ATTR(mode_wave,         0220, NULL, razer_attr_write_mode_wave);
static DEVICE_ATTR(mode_spectrum,     0220, NULL, razer_attr_write_mode_spectrum);
static DEVICE_ATTR(mode_none,         0220, NULL, razer_attr_write_mode_none);
static DEVICE_ATTR(mode_reactive,     0220, NULL, razer_attr_write_mode_reactive);
static DEVICE_ATTR(mode_breath,       0220, NULL, razer_attr_write_mode_breath);
static DEVICE_ATTR(mode_custom,       0220, NULL, razer_attr_write_mode_custom);
static DEVICE_ATTR(mode_static,       0220, NULL, razer_attr_write_mode_static);
static DEVICE_ATTR(temp_clear_row,    0220, NULL, razer_attr_write_temp_clear_row);
static DEVICE_ATTR(set_key_row,       0220, NULL, razer_attr_write_set_key_row);
static DEVICE_ATTR(reset,             0220, NULL, razer_attr_write_reset);
static DEVICE_ATTR(macro_keys,        0220, NULL, razer_attr_write_macro_keys);
static DEVICE_ATTR(set_logo,          0220, NULL, razer_attr_write_set_logo);
static DEVICE_ATTR(set_fn_toggle,     0220, NULL, razer_attr_write_set_fn_toggle);
                                      
static DEVICE_ATTR(test,              0660, razer_attr_read_test, razer_attr_write_test);



static int razer_event(struct hid_device *hdev, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct razer_kbd_device *asc = hid_get_drvdata(hdev);
    const struct razer_key_translation *translation;

    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
    {
        // Skip this if its control (mouse) interface
        return 0;
    }
    
    if (usage->code == RAZER_FN_KEY) {
        asc->fn_on = !!value;
        
        // input_event(field->hidinput->input, usage->type, usage->code, value);
        return 1;
    }
    
    // Do translation, currently translation is only when FN is pressed
    if (asc->fn_on) {
        // Look at https://github.com/torvalds/linux/blob/master/drivers/hid/hid-apple.c#L206 for reversing the FN keys, though blade does that in h/w
        
        translation = find_translation(chroma_keys, usage->code);
        
        if (translation) {
            // translate != NULL, aka a translation is found
            
            if (!(translation->flags & KEY_FLAG_BLOCK)) {
                input_event(field->hidinput->input, usage->type, translation->to, value);
            }
            
            return 1;
        }
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
    
    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
    {

        // If the currently bound device is the control (mouse) interface

        if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL ||
           usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012 ||
           usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013)
        {
            // Razer BlackWidow, BlackWidow 2012, BlackWidow 2013
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_pulsate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_keys);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_game);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_macro);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_macro_effect);
            
        } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016)
        {
            // Razer BlackWidow 2016
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_starlight);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_temp_clear_row);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_set_key_row);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_keys);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_game);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_macro);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_macro_effect);
            
        } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH)
        {
            // Razer Stealth (Laptop)
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_temp_clear_row);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_set_key_row);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_game);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_set_logo);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_set_fn_toggle);
            
        } else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_TARTARUS_CHROMA)
        {
            // Razer Tartarus
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_red);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_green);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_profile_led_blue);
            
        } else // Chroma
        {
            // Razer BlackWidow Chroma, BlackWidow Chroma TE
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_temp_clear_row);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_set_key_row);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_macro_keys);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_game);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_macro);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mode_macro_effect);
        }

        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_set_brightness);

        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_get_firmware_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_get_serial);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_reset);

        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);
    }

    hid_set_drvdata(hdev, dev);

    if(hid_parse(hdev)) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }


    razer_reset(usb_dev);
    usb_disable_autosuspend(usb_dev);
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

    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE)
    {
        // If the currently bound device is the control (mouse) interface

        if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL ||
		   usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012 ||
		   usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013)
		{
			// Razer BlackWidow, BlackWidow 2012, BlackWidow 2013
			device_remove_file(&hdev->dev, &dev_attr_mode_pulsate);         // Pulsate effect, like breathing
			device_remove_file(&hdev->dev, &dev_attr_mode_static);          // Static effect
			device_remove_file(&hdev->dev, &dev_attr_macro_keys);           // Enable macro keys
			device_remove_file(&hdev->dev, &dev_attr_mode_game);            // Enable game mode & LED
			device_remove_file(&hdev->dev, &dev_attr_mode_macro);           // Enable macro LED
			device_remove_file(&hdev->dev, &dev_attr_mode_macro_effect);    // Change macro LED effect (static, flashing)
			
			
			
		} else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016)
		{
			// Razer BlackWidow 2016
			device_remove_file(&hdev->dev, &dev_attr_mode_wave);            // Wave effect
			device_remove_file(&hdev->dev, &dev_attr_mode_starlight);       // Starlight effect
			device_remove_file(&hdev->dev, &dev_attr_mode_none);            // No effect
			device_remove_file(&hdev->dev, &dev_attr_mode_reactive);        // Reactive effect
			device_remove_file(&hdev->dev, &dev_attr_mode_breath);          // Breathing effect
			device_remove_file(&hdev->dev, &dev_attr_mode_static);          // Static effect
			device_remove_file(&hdev->dev, &dev_attr_mode_custom);          // Custom effect
			device_remove_file(&hdev->dev, &dev_attr_temp_clear_row);       // Clear row
			device_remove_file(&hdev->dev, &dev_attr_set_key_row);          // Set LED matrix
			device_remove_file(&hdev->dev, &dev_attr_macro_keys);           // Enable macro keys
			device_remove_file(&hdev->dev, &dev_attr_mode_game);            // Enable game mode & LED
			device_remove_file(&hdev->dev, &dev_attr_mode_macro);           // Enable macro LED
			device_remove_file(&hdev->dev, &dev_attr_mode_macro_effect);    // Change macro LED effect (static, flashing)
			
			
		} else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_BLADE_STEALTH)
		{
			// Razer Stealth (Laptop)
			device_remove_file(&hdev->dev, &dev_attr_mode_wave);            // Wave effect
			device_remove_file(&hdev->dev, &dev_attr_mode_spectrum);        // Spectrum effect
			device_remove_file(&hdev->dev, &dev_attr_mode_none);            // No effect
			device_remove_file(&hdev->dev, &dev_attr_mode_reactive);        // Reactive effect
			device_remove_file(&hdev->dev, &dev_attr_mode_breath);          // Breathing effect
			device_remove_file(&hdev->dev, &dev_attr_mode_static);          // Static effect
			device_remove_file(&hdev->dev, &dev_attr_mode_custom);          // Custom effect
			device_remove_file(&hdev->dev, &dev_attr_temp_clear_row);       // Clear row
			device_remove_file(&hdev->dev, &dev_attr_set_key_row);          // Set LED matrix
			device_remove_file(&hdev->dev, &dev_attr_mode_game);            // Enable game mode & LED
			device_remove_file(&hdev->dev, &dev_attr_set_logo);             // Enable/Disable the logo
			device_remove_file(&hdev->dev, &dev_attr_set_fn_toggle);        // Sets wether FN is requires for F-Keys
			
			
		} else if(usb_dev->descriptor.idProduct == USB_DEVICE_ID_RAZER_TARTARUS_CHROMA) {
			// Razer Tartarus Chroma
			device_remove_file(&hdev->dev, &dev_attr_mode_spectrum);        // Spectrum effect
			device_remove_file(&hdev->dev, &dev_attr_mode_static);          // Static effect
			device_remove_file(&hdev->dev, &dev_attr_mode_breath);          // Breathing effect
			device_remove_file(&hdev->dev, &dev_attr_mode_none);            // No effect
			device_remove_file(&hdev->dev, &dev_attr_profile_led_red);      // Profile/Macro LED Red
			device_remove_file(&hdev->dev, &dev_attr_profile_led_green);    // Profile/Macro LED Green
			device_remove_file(&hdev->dev, &dev_attr_profile_led_blue);     // Profile/Macro LED Blue
		} else
		{
			// Razer BlackWidow Chroma, BlackWidow Chroma TE
			device_remove_file(&hdev->dev, &dev_attr_mode_wave);            // Wave effect
			device_remove_file(&hdev->dev, &dev_attr_mode_spectrum);        // Spectrum effect
			device_remove_file(&hdev->dev, &dev_attr_mode_none);            // No effect
			device_remove_file(&hdev->dev, &dev_attr_mode_reactive);        // Reactive effect
			device_remove_file(&hdev->dev, &dev_attr_mode_breath);          // Breathing effect
			device_remove_file(&hdev->dev, &dev_attr_mode_static);          // Static effect
			device_remove_file(&hdev->dev, &dev_attr_mode_custom);          // Custom effect
			device_remove_file(&hdev->dev, &dev_attr_temp_clear_row);       // Clear row
			device_remove_file(&hdev->dev, &dev_attr_set_key_row);          // Set LED matrix
			device_remove_file(&hdev->dev, &dev_attr_macro_keys);           // Enable macro keys
			device_remove_file(&hdev->dev, &dev_attr_mode_game);            // Enable game mode & LED
			device_remove_file(&hdev->dev, &dev_attr_mode_macro);           // Enable macro LED
			device_remove_file(&hdev->dev, &dev_attr_mode_macro_effect);    // Change macro LED effect (static, flashing)
		}

		device_remove_file(&hdev->dev, &dev_attr_get_firmware_version);     // Get the firmware version
		device_remove_file(&hdev->dev, &dev_attr_get_serial);               // Get serial nubmer
		device_remove_file(&hdev->dev, &dev_attr_mode_static);              // Set static colour
		device_remove_file(&hdev->dev, &dev_attr_reset);                    // TODO REMOVE Reset command
		device_remove_file(&hdev->dev, &dev_attr_set_brightness);           // Gets and sets the brightness
		device_remove_file(&hdev->dev, &dev_attr_test);
		device_remove_file(&hdev->dev, &dev_attr_device_type);              // Get string of device type
        
        if(usb_dev->descriptor.idProduct != USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_X)
        {
            device_remove_file(&hdev->dev, &dev_attr_mode_macro);
            device_remove_file(&hdev->dev, &dev_attr_mode_macro_effect);
        }
    }

    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Device disconnected\n");
}

static int razer_input_mapping(struct hid_device *hdev, struct hid_input *hi, struct hid_field *field, struct hid_usage *usage,    unsigned long **bit, int *max)
{
    const struct razer_key_translation *trans;
    
    if (usage->hid == (HID_UP_CUSTOM | 0x0003)) {
        set_bit(EV_REP, hi->input->evbit);
        hid_map_usage_clear(hi, usage, bit, max, EV_KEY, RAZER_FN_KEY);
        for (trans = chroma_keys; trans->from; trans++) {
            set_bit(trans->to, hi->input->keybit);
        }
        
        return 1;
    }

    return 0;
}

/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ORIGINAL) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2012) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2013) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_ULTIMATE_2016) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_TARTARUS_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_X) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA_TE) },
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
    .input_mapping = razer_input_mapping,
    .event = razer_event,
};

module_hid_driver(razer_kbd_driver);



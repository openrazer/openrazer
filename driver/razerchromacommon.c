#include "razerchromacommon.h"

/*
 * Standard Device Functions
 */

/**
 * Set what mode the device will operate in.
 * 
 * Currently known modes
 * 0x00, 0x00: Normal Mode
 * 0x02, 0x00: Unknown Mode
 * 0x03, 0x00: Driver Mode
 * 
 * 0x02, 0x00 Will make M1-5 and FN emit normal keystrokes. Some sort of factory test mode. Not recommended to be used.
 */
struct razer_report razer_chroma_standard_set_device_mode(unsigned char mode, unsigned char param)
{
	struct razer_report report = get_razer_report(0x00, 0x04, 0x02);
	
	if(mode != 0x00 && mode != 0x03) { // Explicitly blocking the 0x02 mode
		mode = 0x00;
	}
	if(param != 0x00) {
		param = 0x00;
	}
	
	report.arguments[0] = mode;
	report.arguments[1] = param;
	
	return report;
}

/**
 * Get what mode the device is operating in.
 * 
 * Currently known modes
 * 0x00, 0x00: Normal Mode
 * 0x02, 0x00: Unknown Mode
 * 0x03, 0x00: Driver Mode
 * 
 * 0x02, 0x00 Will make M1-5 and FN emit normal keystrokes. Some sort of factory test mode. Not recommended to be used.
 */
struct razer_report razer_chroma_standard_get_device_mode(void)
{
	return get_razer_report(0x00, 0x84, 0x02);
}

/**
 * Get serial from device
 */
struct razer_report razer_chroma_standard_get_serial(void)
{
	return get_razer_report(0x00, 0x82, 0x16);
}

/**
 * Get firmware version from device
 */
struct razer_report razer_chroma_standard_get_firmware_version(void)
{
	return get_razer_report(0x00, 0x81, 0x02);
}

/*
 * Standard Functions
 */
 
/**
 * Set the state of an LED on the device
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    03       03    00  010801                     | SET LED STATE (VARSTR, GAMEMODE, ON)
 * 00     3f    0000   00    03       03    00  010800                     | SET LED STATE (VARSTR, GAMEMODE, OFF)
 */
struct razer_report razer_chroma_standard_set_led_state(unsigned char variable_storage, unsigned char led_id, unsigned char led_state)
{
	struct razer_report report = get_razer_report(0x03, 0x00, 0x03);
	report.arguments[0] = variable_storage;
	report.arguments[1] = led_id;
	report.arguments[2] = clamp_u8(led_state, 0x00, 0x01);
	
	return report;
}

/**
 * Get the state of an LED on the device
 */
struct razer_report razer_chroma_standard_get_led_state(unsigned char variable_storage, unsigned char led_id)
{
	struct razer_report report = get_razer_report(0x03, 0x80, 0x03);
	report.arguments[0] = variable_storage;
	report.arguments[1] = led_id;
	
	return report;
}

/**
 * Set the effect of an LED on the device
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_set_led_effect(unsigned char variable_storage, unsigned char led_id, unsigned char led_effect)
{
	struct razer_report report = get_razer_report(0x03, 0x02, 0x03);
	report.arguments[0] = variable_storage;
	report.arguments[1] = led_id;
	report.arguments[2] = clamp_u8(led_effect, 0x00, 0x05);
		
	return report;
}

/**
 * Get the effect of an LED on the device
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_get_led_effect(unsigned char variable_storage, unsigned char led_id)
{
	struct razer_report report = get_razer_report(0x03, 0x02, 0x03);
	report.arguments[0] = variable_storage;
	report.arguments[1] = led_id;
		
	return report;
}

/**
 * Set the brightness of an LED on the device
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_set_led_brightness(unsigned char variable_storage, unsigned char led_id, unsigned char brightness)
{
	struct razer_report report = get_razer_report(0x03, 0x03, 0x03);
    report.arguments[0] = variable_storage;
    report.arguments[1] = led_id;
    report.arguments[2] = brightness;
    
    return report;
}

/**
 * Get the brightness of an LED on the device
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_get_led_brightness(unsigned char variable_storage, unsigned char led_id)
{
	struct razer_report report = get_razer_report(0x03, 0x83, 0x03);
    report.arguments[0] = variable_storage;
    report.arguments[1] = led_id;
    
    return report;
}







/*
 * Standard Matrix Effects Functions
 */
 
/**
 * Set the effect of the LED matrix to None
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_none(unsigned char variable_storage, unsigned char led_id)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x00; // Effect ID
    
    return report;
}

/**
 * Set the effect of the LED matrix to Wave
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_wave(unsigned char variable_storage, unsigned char led_id, unsigned char wave_direction)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x01; // Effect ID
    report.arguments[0] = clamp_u8(wave_direction, 0x01, 0x02);
    
    return report;
}

/**
 * Set the effect of the LED matrix to Spectrum
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_spectrum(unsigned char variable_storage, unsigned char led_id)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x04; // Effect ID
    
    return report;
}

/**
 * Set the effect of the LED matrix to Reactive
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_reactive(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
	report.arguments[0] = 0x02; // Effect ID
	report.arguments[1] = clamp_u8(speed, 0x01, 0x04); // Time
	report.arguments[2] = rgb1->r; /*rgb color definition*/
	report.arguments[3] = rgb1->g;
	report.arguments[4] = rgb1->b;
    
    return report;
}

/**
 * Set the effect of the LED matrix to Static
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_static(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x06; // Effect ID
    report.arguments[1] = rgb1->r; /*rgb color definition*/
    report.arguments[2] = rgb1->g;
    report.arguments[3] = rgb1->b;
    
    return report;
}

/**
 * Set the effect of the LED matrix to Starlight
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_standard_matrix_effect_starlight_single(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
	
	speed = clamp_u8(speed, 0x01, 0x01); // For now only seen 
	
    report.arguments[0] = 0x19; // Effect ID
    report.arguments[1] = 0x01; // Type one color
    report.arguments[2] = speed; // Speed

    report.arguments[3] = rgb1->r; // Red 1
    report.arguments[4] = rgb1->g; // Green 1
    report.arguments[5] = rgb1->b; // Blue 1

	// For now havent seen any chroma using this, seen the extended version
    report.arguments[6] = 0x00; // Red 2
    report.arguments[7] = 0x00; // Green 2
    report.arguments[8] = 0x00; // Blue 2
	
	return report;
}

/**
 * Set the device to "Breathing" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ??
 * ??
 * ??
 */
struct razer_report razer_chroma_standard_matrix_effect_breathing_random(unsigned char variable_storage, unsigned char led_id)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x08);
	report.arguments[0] = 0x03; // Effect ID
	report.arguments[1] = 0x03; // Breathing type
	
	return report;
}
struct razer_report razer_chroma_standard_matrix_effect_breathing_single(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x08);
	report.arguments[0] = 0x03; // Effect ID
	report.arguments[1] = 0x01; // Breathing type
	report.arguments[2] = rgb1->r;
	report.arguments[3] = rgb1->g;
	report.arguments[4] = rgb1->b;
	
	return report;
}
struct razer_report razer_chroma_standard_matrix_effect_breathing_dual(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1, struct razer_rgb *rgb2)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x08);
	report.arguments[0] = 0x03; // Effect ID
	report.arguments[1] = 0x02; // Breathing type
	report.arguments[2] = rgb1->r;
	report.arguments[3] = rgb1->g;
	report.arguments[4] = rgb1->b;
	report.arguments[5] = rgb2->r;
	report.arguments[6] = rgb2->g;
	report.arguments[7] = rgb2->b;
	
	return report;
}

/**
 * Set the device to "Custom" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ??
 * 
 * Apparently Ultimate2016, Stealth and Stealth2016 need frame id to be 0x00, I dont think its needed (depending on set_custom_frame)
 */
struct razer_report razer_chroma_standard_matrix_effect_custom_frame(void)
{
	struct razer_report report = get_razer_report(0x03, 0x0A, 0x02);
    report.arguments[0] = 0x05; // Effect ID
    report.arguments[1] = 0x01; // Data frame ID
    // report.arguments[1] = 0x01; // Data frame ID
	
	return report;
}


/*
 * Extended Matrix Effects
 */

/**
 * Sets up the extended matrix effect payload
 */
struct razer_report razer_chroma_extended_matrix_effect_base(unsigned char arg_size, unsigned char variable_storage, unsigned char led_id, unsigned char effect_id)
{
	struct razer_report report = get_razer_report(0x0F, 0x02, arg_size);
	report.transaction_id.id = 0x3F;
	
	report.arguments[0] = variable_storage;
	report.arguments[1] = led_id;
	report.arguments[2] = effect_id; // Effect ID
	
	return report;
}

/**
 * Set the device to "None" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    06       0f    02  010500000000  | SET LED MATRIX Effect (VARSTR, Backlight, None 0x00, 0x000000)
 */
struct razer_report razer_chroma_extended_matrix_effect_none(unsigned char variable_storage, unsigned char led_id)
{
	return razer_chroma_extended_matrix_effect_base(0x06, variable_storage, led_id, 0x00);
}

/**
 * Set the device to "Static" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    09       0f    02  010501000001ff0000 | SET LED MATRIX Effect (VARSTR, Backlight, Static 0x01, ? 0x000001, RGB 0xFF0000)
 * 00     3f    0000   00    09       0f    02  01050100000100ff00 | SET LED MATRIX Effect (VARSTR, Backlight, Static 0x01, ? 0x000001, RGB 0x00FF00)
 * 00     3f    0000   00    09       0f    02  010501000001008000 | SET LED MATRIX Effect (VARSTR, Backlight, Static 0x01, ? 0x000001, RGB 0x008000)
 */
struct razer_report razer_chroma_extended_matrix_effect_static(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x09, variable_storage, led_id, 0x01);
	
	report.arguments[5] = 0x01;
	report.arguments[6] = rgb->r;
	report.arguments[7] = rgb->g;
	report.arguments[8] = rgb->b;
	return report;
}

/**
 * Set the device to "Wave" effect
 * 
 * Seems like direction is now 0x00, 0x01 for Left/Right, used to be 0x01, 0x02
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    06       0f    02  010504002800 | SET LED MATRIX Effect (VARSTR, Backlight, Wave 0x04, Dir 0x00, ? 0x2800)
 * 00     3f    0000   00    06       0f    02  010504012800 | SET LED MATRIX Effect (VARSTR, Backlight, Wave 0x04, Dir 0x01, ? 0x2800)
 */
struct razer_report razer_chroma_extended_matrix_effect_wave(unsigned char variable_storage, unsigned char led_id, unsigned char direction)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x06, variable_storage, led_id, 0x04);
	
	direction = clamp_u8(direction, 0x00, 0x01);

	report.arguments[3] = direction;	
	report.arguments[4] = 0x28; // Unknown
	return report;
}

/**
 * Set the device to "Starlight" effect
 * 
 * Speed is 0x01 - 0x03
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    06       0f    02  010507000100             | SET LED MATRIX Effect (VARSTR, Backlight, Starlight 0x07, ? 0x00, Speed 0x01, Colours 0x00)
 * 00     3f    0000   00    06       0f    02  010507000200             | SET LED MATRIX Effect (VARSTR, Backlight, Starlight 0x07, ? 0x00, Speed 0x02, Colours 0x00)
 * 00     3f    0000   00    06       0f    02  010507000300             | SET LED MATRIX Effect (VARSTR, Backlight, Starlight 0x07, ? 0x00, Speed 0x03, Colours 0x00)
 * 00     3f    0000   00    09       0f    02  010507000301ff0000       | SET LED MATRIX Effect (VARSTR, Backlight, Starlight 0x07, ? 0x00, Speed 0x03, Colours 0x01, RGB 0xFF0000)
 * 00     3f    0000   00    0c       0f    02  010507000302ff000000ff00 | SET LED MATRIX Effect (VARSTR, Backlight, Starlight 0x07, ? 0x00, Speed 0x03, Colours 0x02, RGB 0xFF0000, RGB 0x00FF00)
 */
struct razer_report razer_chroma_extended_matrix_effect_starlight_random(unsigned char variable_storage, unsigned char led_id, unsigned char speed)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x06, variable_storage, led_id, 0x07);
	
	speed = clamp_u8(speed, 0x01, 0x03);
	
	report.arguments[4] = speed;
	return report;
}
struct razer_report razer_chroma_extended_matrix_effect_starlight_single(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x09, variable_storage, led_id, 0x07);
	
	speed = clamp_u8(speed, 0x01, 0x03);
	
	report.arguments[4] = speed;
	report.arguments[5] = 0x01;
	report.arguments[6] = rgb1->r;
	report.arguments[7] = rgb1->g;
	report.arguments[8] = rgb1->b;
	
	return report;
}
struct razer_report razer_chroma_extended_matrix_effect_starlight_dual(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1, struct razer_rgb *rgb2)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x0C, variable_storage, led_id, 0x07);
	
	speed = clamp_u8(speed, 0x01, 0x03);
	
	report.arguments[4] = speed;
	report.arguments[5] = 0x02;
	report.arguments[6] = rgb1->r;
	report.arguments[7] = rgb1->g;
	report.arguments[8] = rgb1->b;
	report.arguments[9] = rgb2->r;
	report.arguments[10] = rgb2->g;
	report.arguments[11] = rgb2->b;
	
	return report;
}

/**
 * Set the device to "Spectrum" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    06       0f    02  010503000000 | SET LED MATRIX Effect (VARSTR, Backlight, Spectrum 0x03, 0x000000)
 */
struct razer_report razer_chroma_extended_matrix_effect_spectrum(unsigned char variable_storage, unsigned char led_id)
{
	return razer_chroma_extended_matrix_effect_base(0x06, variable_storage, led_id, 0x03);
}

/**
 * Set the device to "Reactive" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    09       0f    02  010505000101ffff00 | SET LED MATRIX Effect (VARSTR, Backlight, Reactive 0x05, ? 0x00, Speed 0x01, Colours 0x01, RGB 0xFFFF00)
 * 00     3f    0000   00    09       0f    02  010505000101ff0000 | SET LED MATRIX Effect (VARSTR, Backlight, Reactive 0x05, ? 0x00, Speed 0x02, Colours 0x01, RGB 0xFF0000)
 * 00     3f    0000   00    09       0f    02  010505000301ff0000 | SET LED MATRIX Effect (VARSTR, Backlight, Reactive 0x05, ? 0x00, Speed 0x03, Colours 0x01, RGB 0xFF0000)
 * 00     3f    0000   00    09       0f    02  010505000401ff0000 | SET LED MATRIX Effect (VARSTR, Backlight, Reactive 0x05, ? 0x00, Speed 0x04, Colours 0x01, RGB 0xFF0000)
 */
struct razer_report razer_chroma_extended_matrix_effect_reactive(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x09, variable_storage, led_id, 0x05);
	
	speed = clamp_u8(speed, 0x01, 0x04);
	
	report.arguments[4] = speed;
	report.arguments[5] = 0x01;
	report.arguments[6] = rgb->r;
	report.arguments[7] = rgb->g;
	report.arguments[8] = rgb->b;
	
	return report;
}

/**
 * Set the device to "Breathing" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    09       0f    02  01050201000100ff00       | SET LED MATRIX Effect (VARSTR, Backlight, Breathing 0x02, Colours 0x01, ? 0x00, Colours 0x01, RGB 0x00FF00)
 * 00     3f    0000   00    0c       0f    02  01050202000200ff00ff0000 | SET LED MATRIX Effect (VARSTR, Backlight, Breathing 0x02, Colours 0x02, ? 0x00, Colours 0x02, RGB 0x00FF00, RGB 0xFF0000)
 * 00     3f    0000   00    06       0f    02  010502000000             | SET LED MATRIX Effect (VARSTR, Backlight, Breathing 0x02, Colours 0x00, ? 0x0000)
 */
struct razer_report razer_chroma_extended_matrix_effect_breathing_random(unsigned char variable_storage, unsigned char led_id)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x06, variable_storage, led_id, 0x02);	
	return report;
}
struct razer_report razer_chroma_extended_matrix_effect_breathing_single(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x09, variable_storage, led_id, 0x02);
	
	report.arguments[3] = 0x01;
	report.arguments[5] = 0x01;
	
	report.arguments[6] = rgb1->r;
	report.arguments[7] = rgb1->g;
	report.arguments[8] = rgb1->b;
	
	return report;
}
struct razer_report razer_chroma_extended_matrix_effect_breathing_dual(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1, struct razer_rgb *rgb2)
{
	struct razer_report report = razer_chroma_extended_matrix_effect_base(0x0C, variable_storage, led_id, 0x02);
	
	report.arguments[3] = 0x02;
	report.arguments[5] = 0x02;
	
	report.arguments[6] = rgb1->r;
	report.arguments[7] = rgb1->g;
	report.arguments[8] = rgb1->b;
	report.arguments[9] = rgb2->r;
	report.arguments[10] = rgb2->g;
	report.arguments[11] = rgb2->b;
	
	return report;
}

/**
 * Set the device to "Custom" effect
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * 00     3f    0000   00    0c       0f    02  000008000000000000000000   | DRAW LED MATRIX Frame
 */
struct razer_report razer_chroma_extended_matrix_effect_custom_frame(void)
{
	return razer_chroma_extended_matrix_effect_base(0x0C, 0x00, 0x00, 0x08);
}

/**
 * Set the device brightness
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ??
 */
struct razer_report razer_chroma_extended_matrix_brightness(unsigned char variable_storage, unsigned char led_id, unsigned char brightness)
{
	struct razer_report report = get_razer_report(0x0F, 0x04, 0x03);
	report.transaction_id.id = 0x3F;
	
	report.arguments[0] = variable_storage;
	report.arguments[1] = led_id;
	report.arguments[2] = brightness;
	
	return report;
} // TODO does it have an 0x84? to check brightness


/*
 * Misc Functions
 */
/**
 * Toggled wether F1-12 act as F1-12 or if they act as the function options (without Fn pressed)
 * 
 * If 0 should mean that the F-keys work as normal F-keys
 * If 1 should mean that the F-keys act as if the FN key is held
 */
struct razer_report razer_chroma_misc_fn_key_toggle(unsigned char state)
{
      struct razer_report report = get_razer_report(0x02, 0x06, 0x02);
      report.arguments[0] = 0x00; // ?? Variable storage maybe
      report.arguments[1] = clamp_u8(state, 0x00, 0x01); // State
	
	return report;
}

/**
 * Set the brightness of an LED on the device
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_misc_set_blade_brightness(unsigned char brightness)
{
	struct razer_report report = get_razer_report(0x0E, 0x04, 0x02);
    report.arguments[0] = 0x01;
    report.arguments[1] = brightness;
    
    return report;
}

/**
 * Get the brightness of an LED on the device
 * 
 * Status Trans Packet Proto DataSize Class CMD Args
 * ? TODO fill this
 */
struct razer_report razer_chroma_misc_get_blade_brightness(void)
{
	struct razer_report report = get_razer_report(0x0E, 0x84, 0x02);
    report.arguments[0] = 0x01;
    
    return report;
}





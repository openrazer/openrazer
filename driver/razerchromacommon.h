#ifndef DRIVER_RAZERCHROMACOMMON_H_
#define DRIVER_RAZERCHROMACOMMON_H_

#include "razercommon.h"

/*
 * Standard Device Functions
 */
struct razer_report razer_chroma_standard_set_device_mode(unsigned char mode, unsigned char param);
struct razer_report razer_chroma_standard_get_device_mode(void);

struct razer_report razer_chroma_standard_get_serial(void);

struct razer_report razer_chroma_standard_get_firmware_version(void);


/*
 * Standard LED Functions
 */
struct razer_report razer_chroma_standard_set_led_state(unsigned char variable_storage, unsigned char led_id, unsigned char led_state);
struct razer_report razer_chroma_standard_get_led_state(unsigned char variable_storage, unsigned char led_id);

struct razer_report razer_chroma_standard_set_led_rgb(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1);
struct razer_report razer_chroma_standard_get_led_rgb(unsigned char variable_storage, unsigned char led_id);

struct razer_report razer_chroma_standard_set_led_effect(unsigned char variable_storage, unsigned char led_id, unsigned char led_effect);
struct razer_report razer_chroma_standard_get_led_effect(unsigned char variable_storage, unsigned char led_id);

struct razer_report razer_chroma_standard_set_led_brightness(unsigned char variable_storage, unsigned char led_id, unsigned char brightness);
struct razer_report razer_chroma_standard_get_led_brightness(unsigned char variable_storage, unsigned char led_id);

/*
 * Standard Matrix Effects Functions
 */
struct razer_report razer_chroma_standard_matrix_effect_none(unsigned char variable_storage, unsigned char led_id);
struct razer_report razer_chroma_standard_matrix_effect_wave(unsigned char variable_storage, unsigned char led_id, unsigned char wave_direction);
struct razer_report razer_chroma_standard_matrix_effect_spectrum(unsigned char variable_storage, unsigned char led_id);
struct razer_report razer_chroma_standard_matrix_effect_reactive(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1);
struct razer_report razer_chroma_standard_matrix_effect_static(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1);
struct razer_report razer_chroma_standard_matrix_effect_starlight_single(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1);
struct razer_report razer_chroma_standard_matrix_effect_breathing_random(unsigned char variable_storage, unsigned char led_id);
struct razer_report razer_chroma_standard_matrix_effect_breathing_single(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1);
struct razer_report razer_chroma_standard_matrix_effect_breathing_dual(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1, struct razer_rgb *rgb2);
struct razer_report razer_chroma_standard_matrix_effect_custom_frame(unsigned char variable_storage);
struct razer_report razer_chroma_standard_matrix_set_custom_frame(unsigned char row_index, unsigned char start_col, unsigned char stop_col, unsigned char *rgb_data);


/* 
 * Extended Matrix Effects Functions
 * 
 * Class 0x0F
 * Trans 0x3F (Dev 0b001 Game Controller 1, Trans 0b11111)
 */
struct razer_report razer_chroma_extended_matrix_effect_none(unsigned char variable_storage, unsigned char led_id);
struct razer_report razer_chroma_extended_matrix_effect_static(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb);
struct razer_report razer_chroma_extended_matrix_effect_wave(unsigned char variable_storage, unsigned char led_id, unsigned char direction);
struct razer_report razer_chroma_extended_matrix_effect_starlight_random(unsigned char variable_storage, unsigned char led_id, unsigned char speed);
struct razer_report razer_chroma_extended_matrix_effect_starlight_single(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1);
struct razer_report razer_chroma_extended_matrix_effect_starlight_dual(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1, struct razer_rgb *rgb2);
struct razer_report razer_chroma_extended_matrix_effect_spectrum(unsigned char variable_storage, unsigned char led_id);
struct razer_report razer_chroma_extended_matrix_effect_reactive(unsigned char variable_storage, unsigned char led_id, unsigned char speed, struct razer_rgb *rgb1);
struct razer_report razer_chroma_extended_matrix_effect_breathing_random(unsigned char variable_storage, unsigned char led_id);
struct razer_report razer_chroma_extended_matrix_effect_breathing_single(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1);
struct razer_report razer_chroma_extended_matrix_effect_breathing_dual(unsigned char variable_storage, unsigned char led_id, struct razer_rgb *rgb1, struct razer_rgb *rgb2);
struct razer_report razer_chroma_extended_matrix_effect_custom_frame(void);
struct razer_report razer_chroma_extended_matrix_brightness(unsigned char variable_storage, unsigned char led_id, unsigned char brightness);
struct razer_report razer_chroma_extended_matrix_set_custom_frame(unsigned char row_index, unsigned char start_col, unsigned char stop_col, unsigned char *rgb_data);

// TODO Custom Frame


/*
 * Misc Functions
 */
struct razer_report razer_chroma_misc_fn_key_toggle(unsigned char state);

struct razer_report razer_chroma_misc_set_blade_brightness(unsigned char brightness);
struct razer_report razer_chroma_misc_get_blade_brightness(void);

struct razer_report razer_chroma_misc_one_row_set_custom_frame(unsigned char start_col, unsigned char stop_col, unsigned char *rgb_data);

struct razer_report razer_chroma_misc_get_battery_level(void);
struct razer_report razer_chroma_misc_get_charging_status(void);

struct razer_report razer_chroma_misc_set_dock_charge_type(unsigned char charge_type);

struct razer_report razer_chroma_misc_get_polling_rate(void);
struct razer_report razer_chroma_misc_set_polling_rate(unsigned short polling_rate);

struct razer_report razer_chroma_misc_get_dock_brightness(void);
struct razer_report razer_chroma_misc_set_dock_brightness(unsigned char brightness);

struct razer_report razer_chroma_misc_set_dpi_xy(unsigned char variable_storage, unsigned short dpi_x,unsigned short dpi_y);
struct razer_report razer_chroma_misc_set_idle_time(unsigned short idle_time);
struct razer_report razer_chroma_misc_set_low_battery_threshold(unsigned char battery_threshold);

#endif

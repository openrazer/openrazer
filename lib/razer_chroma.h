/* 
 * razer_chroma_drivers - a driver/tools collection for razer chroma devices
 * (c) 2015 by Tim Theede aka Pez2001 <pez2001@voyagerproject.de> / vp
 *
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
 *
 */
#ifndef _RAZER_CHROMA_H_
#define _RAZER_CHROMA_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>

#include "list.h"
#include "razer_string.h"


/*
#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_image.h"
*/


#define PI 3.1415926535897932384626433832795

//TODO move to common.h
#define RAZER_VENDOR_ID 0x1532
#define RAZER_BLACKWIDOW_CHROMA_PRODUCT_ID 0x203
#define RAZER_BLACKWIDOW_CHROMA_TE_PRODUCT_ID 0x209
#define RAZER_BLACKWIDOW_OLD_PRODUCT_ID 0x011B
#define RAZER_BLACKWIDOW_ULTIMATE_2012_PRODUCT_ID 0x010D
#define RAZER_BLACKWIDOW_ULTIMATE_2013_PRODUCT_ID 0x011A
#define RAZER_BLACKWIDOW_ULTIMATE_2016_PRODUCT_ID 0x0214
#define RAZER_FIREFLY_PRODUCT_ID 0x0c00
#define RAZER_MAMBA_CHROMA_PRODUCT_ID 0x0045
#define RAZER_MAMBA_CHROMA_TE_PRODUCT_ID 0x0046
#define RAZER_STEALTH_PRODUCT_ID 0x0205


#define RAZER_ROW_LENGTH 22
#define RAZER_ROWS_NUM 6


#define RAZER_KEY_CLASS_LETTERS 1
#define RAZER_KEY_CLASS_NUMBERS 2
#define RAZER_KEY_CLASS_FUNCTION_KEYS 3
#define RAZER_KEY_CLASS_MACRO_KEYS 4
#define RAZER_KEY_CLASS_LEFT_CONTROLS 5
#define RAZER_KEY_CLASS_RIGHT_CONTROLS 6
#define RAZER_KEY_CLASS_NUMPAD_NUMBERS 7
#define RAZER_KEY_CLASS_NUMPAD_OPERATIONS 8
#define RAZER_KEY_CLASS_NUMPAD_CONTROLS 9
#define RAZER_KEY_CLASS_ARROWS 10
#define RAZER_KEY_CLASS_POSITION_CONTROLS 11
#define RAZER_KEY_CLASS_SYSTEM_CONTROLS 12
#define RAZER_KEY_CLASS_UNKNOWN 13





#ifdef __cplusplus
extern "C"{
#endif 




struct razer_pos
{
	int x,y;
};

struct razer_rgb 
{
	unsigned char r,g,b;
};

struct razer_hsl
{
	float h,s,l;
};

struct razer_rgb_row
{
	unsigned char row_index;
	struct razer_rgb *column;
	unsigned char columns_num;
	//struct razer_rgb column[RAZER_ROW_LENGTH];
};

struct razer_rgb_frame
{
	//struct razer_rgb_row rows[RAZER_ROWS_NUM];
	unsigned char columns_num;
	unsigned char rows_num;
	struct razer_rgb_row **rows;
	int update_mask;
};

/*struct razer_leds
{
	//struct razer_rgb_row rows[RAZER_ROWS_NUM];
	unsigned char rows_num;
	struct razer_rgb_row **rows;
	int update_mask;
	//long heatmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];
	//int lockmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];//sets to effect id if locked by effect
	//int pushedmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];//all keys pressed will be set 1 (needs razer_update calls to work)
};
*/

struct razer_led_locks
{
	int columns_num;
	int rows_num;
	int *lockmap;
};

struct razer_keys_set
{
	int num;
	unsigned char *keys;/*buffer to keycodes?ascii? */
};


struct razer_chroma;
struct razer_chroma_event;
typedef int (*razer_event_handler)(struct razer_chroma *chroma,struct razer_chroma_event *event);

#define RAZER_CHROMA_DEVICE_TYPE_KEYBOARD 1
#define RAZER_CHROMA_DEVICE_TYPE_MOUSE 2
#define RAZER_CHROMA_DEVICE_TYPE_MOUSEMAT 3
#define RAZER_CHROMA_DEVICE_TYPE_MISC 4

struct razer_chroma_device 
{
	struct razer_chroma *chroma;
	char *path;
	char *name;
	int type;
	int rows_num;
	int columns_num;
	//TODO keyboard dimensions,number of leds,etc
	struct razer_rgb_frame *leds;//TODO rewrite with leds as basis for all devices
	struct razer_led_locks *locks;

	//driver io files
	char *update_leds_filename;
	FILE *update_leds_file;
	char *custom_mode_filename;
	FILE *custom_mode_file;
	char *breath_mode_filename;
	FILE *breath_mode_file;
	char *game_mode_filename;
	FILE *game_mode_file;
	char *none_mode_filename;
	FILE *none_mode_file;
	char *reactive_mode_filename;
	FILE *reactive_mode_file;
	char *spectrum_mode_filename;
	FILE *spectrum_mode_file;
	char *starlight_mode_filename;
	FILE *starlight_mode_file;
	char *static_mode_filename;
	FILE *static_mode_file;
	char *wave_mode_filename;
	FILE *wave_mode_file;
	char *brightness_filename;
	FILE *brightness_file;
	char *temp_clear_row_filename;
	FILE *temp_clear_row_file;
	char *reset_filename;
	FILE *reset_file;
	char *macro_keys_filename;
	FILE *macro_keys_file;
	char *serial_filename;
	FILE *serial_file;
	void *tag;
};

#define RAZER_CHROMA_INPUT_DEVICE_TYPE_KEYBOARD 1
#define RAZER_CHROMA_INPUT_DEVICE_TYPE_MOUSE 2

struct razer_chroma_input_device 
{
	struct razer_chroma *chroma;
	char *path;/*path to event file*/
	char *name;
	int type;
	//int use_device;
	int input_file;
	unsigned long last_event_ms;
	float event_dt;
};




struct razer_chroma
{

	//char *sys_keyboard_event_path;//default = "/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd";
	//char *sys_mouse_event_path;//default = "/dev/input/event26";

	list *devices;//struct razer_chroma_device
	struct razer_chroma_device *active_device;
	list *input_devices;//struct razer_chroma_input_device

	unsigned long last_update_ms;
	unsigned long update_ms;
	float update_dt;
	razer_event_handler event_handler;
	struct razer_pos last_key_pos;//TODO move to sub struct pointer to pointers
	struct razer_pos key_pos;//or remove
	void *tag;
};


#define RAZER_CHROMA_EVENT_TYPE_KEYBOARD 1
#define RAZER_CHROMA_EVENT_TYPE_MOUSE 2
#define RAZER_CHROMA_EVENT_TYPE_USER 3
#define RAZER_CHROMA_EVENT_TYPE_JOYSTICK 4
#define RAZER_CHROMA_EVENT_TYPE_SYSTEM 5
#define RAZER_CHROMA_EVENT_TYPE_FX 6


#define RAZER_CHROMA_EVENT_SUBTYPE_SYSTEM_DEVICE_ADD 1
#define RAZER_CHROMA_EVENT_SUBTYPE_SYSTEM_DEVICE_REMOVE 2
#define RAZER_CHROMA_EVENT_SUBTYPE_SYSTEM_INIT 3
#define RAZER_CHROMA_EVENT_SUBTYPE_SYSTEM_QUIT 4

#define RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_X_AXIS_MOVEMENT 1
#define RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_Y_AXIS_MOVEMENT 2
#define RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_WHEEL_MOVEMENT 3
//#define RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_WHEEL_DOWN 4
#define RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_BUTTON_UP 4
#define RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_BUTTON_DOWN 5

#define RAZER_CHROMA_EVENT_SUBTYPE_KEYBOARD_KEY_UP 0
#define RAZER_CHROMA_EVENT_SUBTYPE_KEYBOARD_KEY_DOWN 1

#define RAZER_CHROMA_EVENT_BUTTON_LEFT 0
#define RAZER_CHROMA_EVENT_BUTTON_MIDDLE 1
#define RAZER_CHROMA_EVENT_BUTTON_RIGHT 2
#define RAZER_CHROMA_EVENT_BUTTON_EXTRA 3

/*struct razer_chroma_joystick_event_values
{
  long rel_x;
  long rel_y;
  int buttons_mask;
};


struct razer_chroma_mouse_event_values
{
  long rel_x;
  long rel_y;
  int buttons_mask;
};

struct razer_chroma_keyboard_event_values
{
  int keycode;
  int pressed;
};
*/

struct razer_chroma_event
{
  int type;
  int sub_type;
  char *key;
  unsigned long long value;
  /*union 
  {
	struct razer_chroma_keyboard_event_values keyboard;
	struct razer_chroma_mouse_event_values mouse;
  }values;
  */
};



//handler can be NULL
struct razer_chroma *razer_open(razer_event_handler event_handler,void *tag);
void razer_close(struct razer_chroma *chroma);
long razer_get_num_devices(struct razer_chroma *chroma);
void razer_set_active_device_id(struct razer_chroma *chroma,int index);
void razer_set_active_device(struct razer_chroma *chroma,struct razer_chroma_device *device);//easiest way so i dont have to rewrite everything in one shot

void razer_close_device(struct razer_chroma_device *device);
void razer_close_devices(struct razer_chroma *chroma);

void razer_fire_event(struct razer_chroma *chroma,struct razer_chroma_event *event);


int razer_device_enable_macro_keys(struct razer_chroma_device *device);
int razer_enable_macro_keys(struct razer_chroma *chroma);
int razer_device_set_custom_mode(struct razer_chroma_device *device);
int razer_set_custom_mode(struct razer_chroma *chroma);
int razer_device_set_breath_mode(struct razer_chroma_device *device,struct razer_rgb *first_color,struct razer_rgb *second_color);
int razer_set_breath_mode(struct razer_chroma *chroma,struct razer_rgb *first_color,struct razer_rgb *second_color);
int razer_device_set_one_color_breath_mode(struct razer_chroma_device *device,struct razer_rgb *first_color);
int razer_set_one_color_breath_mode(struct razer_chroma *chroma,struct razer_rgb *first_color);
int razer_device_set_random_breath_mode(struct razer_chroma_device *device);
int razer_set_random_breath_mode(struct razer_chroma *chroma);
int razer_device_set_game_mode(struct razer_chroma_device *device,unsigned char enable);
int razer_set_game_mode(struct razer_chroma *chroma,unsigned char enable);
int razer_device_set_none_mode(struct razer_chroma_device *device);
int razer_set_none_mode(struct razer_chroma *chroma);
int razer_device_set_reactive_mode(struct razer_chroma_device *device,unsigned char speed,struct razer_rgb *color);
int razer_set_reactive_mode(struct razer_chroma *chroma,unsigned char speed,struct razer_rgb *color);
int razer_device_set_spectrum_mode(struct razer_chroma_device *device);
int razer_set_spectrum_mode(struct razer_chroma *chroma);
int razer_device_set_starlight_mode(struct razer_chroma_device *device);
int razer_set_starlight_mode(struct razer_chroma *chroma);
int razer_device_set_static_mode(struct razer_chroma_device *device,struct razer_rgb *color);
int razer_set_static_mode(struct razer_chroma *chroma,struct razer_rgb *color);
int razer_device_set_wave_mode(struct razer_chroma_device *device,unsigned char direction);
int razer_set_wave_mode(struct razer_chroma *chroma,unsigned char direction);
int razer_device_reset(struct razer_chroma_device *device);
int razer_reset_mode(struct razer_chroma *chroma);
int razer_device_set_brightness(struct razer_chroma_device *device,unsigned char brightness);
int razer_set_brightness(struct razer_chroma *chroma,unsigned char brightness);
int razer_device_temp_clear_row(struct razer_chroma_device *device);
int razer_temp_clear_row(struct razer_chroma *chroma);
int razer_device_set_key_row(struct razer_chroma_device *device,unsigned char row_index,unsigned char num_colors,struct razer_rgb **colors);
int razer_set_key_row(struct razer_chroma *chroma,unsigned char row_index,unsigned char num_colors,struct razer_rgb **colors);
int razer_device_set_key_row_buffered(struct razer_chroma_device *device,unsigned char *buffer,int buffer_len);
int razer_set_key_row_buffered(struct razer_chroma *chroma,unsigned char *buffer,int buffer_len);
int razer_get_serial(struct razer_chroma *chroma, char* buffer);
int razer_device_get_serial(struct razer_chroma_device *device, char* buffer);
int razer_get_name(struct razer_chroma *chroma, char* buffer);
int razer_device_get_name(struct razer_chroma_device *device, char* buffer);



int razer_set_led_row_buffered(struct razer_chroma *chroma,unsigned char *buffer,int buffer_len);


void razer_update(struct razer_chroma *chroma);
void razer_set_event_handler(struct razer_chroma *chroma,razer_event_handler handler);
unsigned long razer_get_ticks();
void razer_frame_limiter(struct razer_chroma *chroma,int fps);

struct razer_rgb_frame *razer_create_rgb_frame(int columns_num,int rows_num);
void razer_free_rgb_frame(struct razer_rgb_frame *frame);


int razer_set_custom_mode(struct razer_chroma *chroma);
//int razer_update_keys(struct razer_chroma *chroma,struct razer_keys *keys);
int razer_device_update_leds(struct razer_chroma_device *device, struct razer_rgb_frame *frame);
int razer_update_leds(struct razer_chroma *chroma,struct razer_rgb_frame *frame);
void razer_clear_frame(struct razer_rgb_frame *frame);
//char *razer_get_device_path();


void razer_copy_rows(struct razer_rgb_row *src_rows,struct razer_rgb_row *dst_rows,int update_mask,int use_update_mask);
void razer_init_frame(struct razer_rgb_frame *frame);
//void razer_init_keys(struct razer_keys *keys);


void razer_set_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color);
void razer_add_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color);
void razer_sub_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color);
void razer_mix_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color,float opacity);
void razer_mix_frames(struct razer_rgb_frame *dst_frame,struct razer_rgb_frame *src_frame,float opacity);

void razer_set_frame_row(struct razer_rgb_frame *frame,int row_index,struct razer_rgb *color);
void razer_add_frame_row(struct razer_rgb_frame *frame,int row_index,struct razer_rgb *color);
void razer_sub_frame_row(struct razer_rgb_frame *frame,int row_index,struct razer_rgb *color);
void razer_set_led(struct razer_rgb_frame *frame,int column_index,int row_index,struct razer_rgb *color);
void razer_set_led_pos(struct razer_rgb_frame *frame,struct razer_pos *pos,struct razer_rgb *color);
void razer_clear_all(struct razer_rgb_frame *frame);
void razer_set_all(struct razer_rgb_frame *frame,struct razer_rgb *color);



void release_locks(struct razer_chroma_device *device);
void razer_copy_pos(struct razer_pos *src, struct razer_pos *dst);

float hue2rgb(float p,float q,float t);
void hsl2rgb(struct razer_hsl *hsl,struct razer_rgb *rgb);
void rgb_from_hue(float percentage,float start_hue,float end_hue,struct razer_rgb *color);
unsigned char rgb_clamp(int v);
void rgb_add(struct razer_rgb *dst,struct razer_rgb *src);
void rgb_mix(struct razer_rgb *dst,struct razer_rgb *src,float factor);
struct razer_rgb *rgb_create(unsigned char r,unsigned char g,unsigned char b);
struct razer_rgb *rgb_copy(struct razer_rgb *color);
struct razer_pos *razer_pos_copy(struct razer_pos *pos);
void rgb_mix_into(struct razer_rgb *dst,struct razer_rgb *src_a,struct razer_rgb *src_b,float dst_opacity);


void razer_convert_keycode_to_pos(int keycode,struct razer_pos *pos);
//void razer_convert_pos_to_keycode(struct razer_pos *pos,int *keycode);
void razer_convert_ascii_to_pos(unsigned char letter,struct razer_pos *pos);
int razer_get_key_class(int keycode);


//void set_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
//void add_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
//void sub_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
//void set_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
//void add_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
//void sub_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
//void razer_set_key(struct razer_keys *keys,int column_index,int row_index,struct razer_rgb *color);
//void razer_set_key_pos(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color);
//void razer_clear_all(struct razer_keys *keys);
//void razer_set_all(struct razer_keys *keys,struct razer_rgb *color);

//void sub_heatmap(struct razer_keys *keys,int heatmap_reduction_amount);
void razer_draw_line(struct razer_rgb_frame *frame,struct razer_pos *a,struct razer_pos *b,struct razer_rgb *color);
void razer_draw_circle(struct razer_rgb_frame *frame,struct razer_pos *pos,int radius,struct razer_rgb *color);
void razer_draw_ring(struct razer_rgb_frame *frame,struct razer_pos *pos,struct razer_rgb *color);


void write_to_device_file(char *device_path, char *buffer, int buffer_length);
void read_from_device_file(char *device_path, char *buffer, int buffer_length);

//list of last keystrokes
//time since hit /hitstamps

double deg2rad(double degree);
double rad2deg(double rad);
double pos_angle_radians(struct razer_pos *src,struct razer_pos *dst);


//void capture_keys(struct razer_keys *keys,SDL_Renderer *renderer,SDL_Window *window,SDL_Texture *tex);

#ifdef __cplusplus
}
#endif

#endif

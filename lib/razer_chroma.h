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


/*
#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_image.h"
*/


#define PI 3.1415926535897932384626433832795

#define RAZER_VENDOR_ID 0x1532
#define RAZER_BLACKWIDOW_CHROMA_PRODUCT_ID 0x203


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
	struct razer_rgb column[RAZER_ROW_LENGTH];
};

struct razer_rgb_frame
{
	struct razer_rgb_row rows[RAZER_ROWS_NUM];
	int update_mask;
};

struct razer_keys
{
	struct razer_rgb_row rows[RAZER_ROWS_NUM];
	int update_mask;
	long heatmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];
	int lockmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];//sets to effect id if locked by effect
	int pushedmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];//all keys pressed will be set 1 (needs razer_update calls to work)
};

struct razer_keys_locks
{
	int lockmap[RAZER_ROWS_NUM][RAZER_ROW_LENGTH];
};

struct razer_keys_set
{
	int num;
	unsigned char *keys;/*buffer to keycodes?ascii? */
};


struct razer_chroma;
struct razer_chroma_event;
//typedef int (*razer_input_handler)(struct razer_chroma *chroma,int keycode,int pressed);
typedef int (*razer_input_handler)(struct razer_chroma *chroma,struct razer_chroma_event *event);

struct razer_chroma
{
	char *device_path;
	char *update_keys_filename;
	char *custom_mode_filename;
	FILE *custom_mode_file;
	FILE *update_keys_file;
	int keyboard_input_file;
	int mouse_input_file;
	unsigned long last_update_ms;
	unsigned long update_ms;
	unsigned long last_key_event_ms;
	float update_dt;
	float key_event_dt;
	struct razer_keys *keys;
	razer_input_handler input_handler;
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
  void *value;
  /*union 
  {
	struct razer_chroma_keyboard_event_values keyboard;
	struct razer_chroma_mouse_event_values mouse;
  }values;
  */
};


char *str_CreateEmpty(void);
char *str_Copy(char *src);
char *str_Cat(char *a,char *b);
char *str_CatFree(char *a,char *b);
char *str_FromLong(long i);
char *str_FromDouble(double d);



struct razer_chroma *razer_open(void);
void razer_close(struct razer_chroma *chroma);
void razer_update(struct razer_chroma *chroma);
void razer_set_input_handler(struct razer_chroma *chroma,razer_input_handler handler);
unsigned long razer_get_ticks();
void razer_frame_limiter(struct razer_chroma *chroma,int fps);

struct razer_rgb_frame *razer_create_rgb_frame(void);
void razer_free_rgb_frame(struct razer_rgb_frame *frame);


void razer_set_custom_mode(struct razer_chroma *chroma);
void razer_update_keys(struct razer_chroma *chroma,struct razer_keys *keys);
void razer_update_frame(struct razer_chroma *chroma,struct razer_rgb_frame *frame);
void razer_clear_frame(struct razer_rgb_frame *frame);
char *razer_get_device_path();


void razer_copy_rows(struct razer_rgb_row *src_rows,struct razer_rgb_row *dst_rows,int update_mask,int use_update_mask);
void razer_init_frame(struct razer_rgb_frame *frame);
void razer_init_keys(struct razer_keys *keys);


void razer_set_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color);
void razer_add_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color);
void razer_sub_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color);
void razer_mix_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color,float opacity);
void razer_mix_frames(struct razer_rgb_frame *dst_frame,struct razer_rgb_frame *src_frame,float opacity);



void release_locks(struct razer_keys_locks *locks);
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


void set_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void add_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void sub_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void set_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void add_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void sub_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void razer_set_key(struct razer_keys *keys,int column_index,int row_index,struct razer_rgb *color);
void razer_set_key_pos(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color);
void razer_clear_all(struct razer_keys *keys);
void razer_set_all(struct razer_keys *keys,struct razer_rgb *color);
void sub_heatmap(struct razer_keys *keys,int heatmap_reduction_amount);
void draw_circle(struct razer_keys *keys,struct razer_pos *pos,int radius,struct razer_rgb *color);
void draw_ring(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color);


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

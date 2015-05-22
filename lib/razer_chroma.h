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



#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_opengl.h"
#include "SDL2/SDL_image.h"



#define PI 3.1415926535897932384626433832795

#define RAZER_VENDOR_ID 0x1532
#define RAZER_BLACKWIDOW_CHROMA_PRODUCT_ID 0x203


#define RAZER_ROW_LENGTH 22
#define RAZER_ROWS_NUM 6




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

typedef int (*razer_input_handler)(struct razer_chroma *chroma,int keycode,int pressed);

struct razer_chroma
{
	char *device_path;
	char *update_keys_filename;
	char *custom_mode_filename;
	FILE *custom_mode_file;
	FILE *update_keys_file;
	int input_file;
	long last_update_ms;
	long update_ms;
	long last_key_event_ms;
	float update_dt;
	float key_event_dt;
	struct razer_keys *keys;
	razer_input_handler input_handler;
	struct razer_pos last_key_pos;//TODO move to sub struct pointer to pointers
	struct razer_pos key_pos;//or remove
};



int razer_open(struct razer_chroma *chroma);
void razer_close(struct razer_chroma *chroma);
void razer_update(struct razer_chroma *chroma);
void razer_set_input_handler(struct razer_chroma *chroma,razer_input_handler handler);
unsigned long razer_get_ticks();


void razer_set_custom_mode(struct razer_chroma *chroma);
void razer_update_keys(struct razer_chroma *chroma,struct razer_keys *keys);
char *razer_get_device_path();



void release_locks(struct razer_keys_locks *locks);
float hue2rgb(float p,float q,float t);
void hsl2rgb(struct razer_hsl *hsl,struct razer_rgb *rgb);
void rgb_from_hue(float percentage,float start_hue,float end_hue,struct razer_rgb *color);
unsigned char rgb_clamp(int v);
void rgb_add(struct razer_rgb *dst,struct razer_rgb *src);
void rgb_mix(struct razer_rgb *dst,struct razer_rgb *src,float factor);





void convert_keycode_to_pos(int keycode,struct razer_pos *pos);
void convert_pos_to_keycode(struct razer_pos *pos,int *keycode);
void convert_ascii_to_pos(unsigned char letter,struct razer_pos *pos);
void set_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void add_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void sub_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color);
void set_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void add_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void sub_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color);
void set_key(struct razer_keys *keys,int column_index,int row_index,struct razer_rgb *color);
void set_key_pos(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color);
void clear_all(struct razer_keys *keys);
void set_all(struct razer_keys *keys,struct razer_rgb *color);
void sub_heatmap(struct razer_keys *keys,int heatmap_reduction_amount);
void draw_circle(struct razer_keys *keys,struct razer_pos *pos,int radius,struct razer_rgb *color);
void draw_ring(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color);


//list of last keystrokes
//time since hit /hitstamps

double deg2rad(double degree);
double rad2deg(double rad);
double pos_angle_radians(struct razer_pos *src,struct razer_pos *dst);


//void capture_keys(struct razer_keys *keys,SDL_Renderer *renderer,SDL_Window *window,SDL_Texture *tex);



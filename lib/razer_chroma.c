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
#include "razer_chroma.h"
#include <syslog.h>

char *razer_sys_hid_devices_path = "/sys/bus/hid/devices/";

char *razer_sys_keyboard_event_default_path = "/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd";
char *razer_sys_mouse_event_default_path = "/dev/input/event22";
//char *razer_sys_mouse_event_path = "/dev/input/mouse0";
//char *razer_sys_mouse_event_path = "/dev/input/mouse2";
//char *razer_sys_mouse_event_path = "/dev/input/by-id/usb-ROCCAT_ROCCAT_Kone_Pure-event-mouse";

char *razer_device_type_pathname = "/device_type";

char *razer_custom_mode_pathname = "/mode_custom";
char *razer_breath_mode_pathname = "/mode_breath";
char *razer_game_mode_pathname = "/mode_game";
char *razer_none_mode_pathname = "/mode_none";
char *razer_reactive_mode_pathname = "/mode_reactive";
char *razer_spectrum_mode_pathname = "/mode_spectrum";
char *razer_starlight_mode_pathname = "/mode_starlight";
char *razer_static_mode_pathname = "/mode_static";
char *razer_wave_mode_pathname = "/mode_wave";
char *razer_brightness_pathname = "/set_brightness";
char *razer_reset_pathname = "/reset";
char *razer_temp_clear_row_pathname = "/temp_clear_row";
char *razer_macro_keys_pathname = "/macro_keys";
char *razer_update_leds_pathname = "/set_key_row";
char *razer_serial_pathname = "/get_serial";

void write_to_device_file(char *device_path, char *buffer, int buffer_length)
{
	FILE* fp;
	fp = fopen(device_path, "wb");
	if(fp != NULL) {
	  fwrite(buffer, sizeof(char), buffer_length, fp);
	  fclose(fp);
	} else
	{
	  printf("Failed to write to %s!\n", device_path);
	}
}

void read_from_device_file(char *device_path, char *buffer, int buffer_length)
{
	FILE* fp;
	fp = fopen(device_path, "rb");
	if(fp != NULL) {
	  fread(buffer, sizeof(char), buffer_length, fp);
	  fclose(fp);
	} else
	{
	  printf("Failed to read to %s!\n", device_path);
	}
}

int razer_open_serial_file(struct razer_chroma_device *device)
{
	device->serial_file=fopen(device->serial_filename,"r");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening serial file:%s\n",device->serial_filename);
	#endif
	if(device->serial_file)
		return(1);
	else
		return(0);
}

void razer_close_serial_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing serial file:%s\n",device->serial_filename);
	#endif
    if(device->serial_file)
    	fclose(device->serial_file);
    device->serial_file = NULL;
}

int razer_open_breath_mode_file(struct razer_chroma_device *device)
{
	device->breath_mode_file=fopen(device->breath_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening breath mode file:%s\n",device->breath_mode_filename);
	#endif
	if(device->breath_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_breath_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing breath mode file:%s\n",device->breath_mode_filename);
	#endif
    if(device->breath_mode_file)
    	fclose(device->breath_mode_file);
    device->breath_mode_file = NULL;
}

int razer_open_game_mode_file(struct razer_chroma_device *device)
{
	device->game_mode_file=fopen(device->game_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening game mode file:%s\n",device->game_mode_filename);
	#endif
	if(device->game_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_game_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing game mode file:%s\n",device->game_mode_filename);
	#endif
    if(device->game_mode_file)
    	fclose(device->game_mode_file);
    device->game_mode_file = NULL;
}

int razer_open_none_mode_file(struct razer_chroma_device *device)
{
	device->none_mode_file=fopen(device->none_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening none mode file:%s\n",device->none_mode_filename);
	#endif
	if(device->none_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_none_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing none mode file:%s\n",device->none_mode_filename);
	#endif
    if(device->none_mode_file)
    	fclose(device->none_mode_file);
    device->none_mode_file = NULL;
}

int razer_open_reactive_mode_file(struct razer_chroma_device *device)
{
	device->reactive_mode_file=fopen(device->reactive_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening reactive mode file:%s\n",device->reactive_mode_filename);
	#endif
	if(device->reactive_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_reactive_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing reactive mode file:%s\n",device->reactive_mode_filename);
	#endif
    if(device->reactive_mode_file)
    	fclose(device->reactive_mode_file);
    device->reactive_mode_file = NULL;
}

int razer_open_spectrum_mode_file(struct razer_chroma_device *device)
{
	device->spectrum_mode_file=fopen(device->spectrum_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening spectrum mode file:%s\n",device->spectrum_mode_filename);
	#endif
	if(device->spectrum_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_spectrum_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing spectrum mode file:%s\n",device->spectrum_mode_filename);
	#endif
    if(device->spectrum_mode_file)
    	fclose(device->spectrum_mode_file);
    device->spectrum_mode_file = NULL;
}

int razer_open_static_mode_file(struct razer_chroma_device *device)
{
	device->static_mode_file=fopen(device->static_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening static mode file:%s\n",device->static_mode_filename);
	#endif
	if(device->static_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_static_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing static mode file:%s\n",device->static_mode_filename);
	#endif
    if(device->static_mode_file)
    	fclose(device->static_mode_file);
    device->static_mode_file = NULL;
}

int razer_open_starlight_mode_file(struct razer_chroma_device *device)
{
	device->starlight_mode_file=fopen(device->starlight_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening starlight mode file:%s\n",device->starlight_mode_filename);
	#endif
	if(device->starlight_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_starlight_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing starlight mode file:%s\n",device->starlight_mode_filename);
	#endif
    if(device->starlight_mode_file)
    	fclose(device->starlight_mode_file);
    device->starlight_mode_file = NULL;
}

int razer_open_wave_mode_file(struct razer_chroma_device *device)
{
	device->wave_mode_file=fopen(device->wave_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening wave mode file:%s\n",device->wave_mode_filename);
	#endif
	if(device->wave_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_wave_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing wave mode file:%s\n",device->wave_mode_filename);
	#endif
    if(device->wave_mode_file)
    	fclose(device->wave_mode_file);
    device->wave_mode_file = NULL;
}

int razer_open_reset_file(struct razer_chroma_device *device)
{
	device->reset_file=fopen(device->reset_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening reset file:%s\n",device->reset_filename);
	#endif
	if(device->reset_file)
		return(1);
	else
		return(0);
}

void razer_close_reset_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing reset file:%s\n",device->reset_filename);
	#endif
    if(device->reset_file)
    	fclose(device->reset_file);
    device->reset_file = NULL;
}

int razer_open_brightness_file(struct razer_chroma_device *device)
{
	device->brightness_file=fopen(device->brightness_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening brightness file:%s\n",device->brightness_filename);
	#endif
	if(device->brightness_file)
		return(1);
	else
		return(0);
}

void razer_close_brightness_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing brightness file:%s\n",device->brightness_filename);
	#endif
    if(device->brightness_file)
    	fclose(device->brightness_file);
    device->brightness_file = NULL;
}

int razer_open_temp_clear_row_file(struct razer_chroma_device *device)
{
	device->temp_clear_row_file=fopen(device->temp_clear_row_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening temp_clear_row file:%s\n",device-temp_clear_row_filename);
	#endif
	if(device->temp_clear_row_file)
		return(1);
	else
		return(0);
}

void razer_close_temp_clear_row_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing temp_clear_row file:%s\n",device->temp_clear_row_filename);
	#endif
    if(device->temp_clear_row_file)
    	fclose(device->temp_clear_row_file);
    device->temp_clear_row_file = NULL;
}

int razer_open_custom_mode_file(struct razer_chroma_device *device)
{
	device->custom_mode_file=fopen(device->custom_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening custom mode file:%s\n",device->custom_mode_filename);
	#endif
	if(device->custom_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_custom_mode_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing custom mode file:%s\n",device->custom_mode_filename);
	#endif
    if(device->custom_mode_file)
    	fclose(device->custom_mode_file);
    device->custom_mode_file = NULL;
}

int razer_open_update_leds_file(struct razer_chroma_device *device)
{
	device->update_leds_file=fopen(device->update_leds_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening update leds file:%s\n",device->update_leds_filename);
	#endif
	if(device->update_leds_file)
		return(1);
	else
		return(0);
}

void razer_close_update_leds_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing update leds file:%s\n",device->update_leds_filename);
	#endif
    if(device->update_leds_file)
    	fclose(device->update_leds_file);
    device->update_leds_file = NULL;
}

int razer_open_macro_keys_file(struct razer_chroma_device *device)
{
	device->macro_keys_file=fopen(device->macro_keys_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening macro keys file:%s\n",device->macro_keys_filename);
	#endif
	if(device->macro_keys_file)
		return(1);
	else
		return(0);
}

void razer_close_macro_keys_file(struct razer_chroma_device *device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing macro keys file:%s\n",device->macro_keys_filename);
	#endif
    if(device->macro_keys_file)
    	fclose(device->macro_keys_file);
    device->macro_keys_file = NULL;
}



/*
 input event system specific files
*/
int razer_open_input_file(struct razer_chroma_input_device *input_device)
{
	input_device->input_file=open(input_device->path,O_RDONLY | O_NONBLOCK | O_NOCTTY | O_NDELAY);
	fcntl(input_device->input_file,F_SETFL,0);
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening input file:%s\n",input_device->path);
	#endif
	if(input_device->input_file)
		return(1);
	else
		return(0);
}

void razer_close_input_file(struct razer_chroma_input_device *input_device)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing input file:%s\n",input_device->path);
	#endif
    if(input_device->input_file)
    	close(input_device->input_file);
    input_device->input_file = 0;
}



/*
void razer_init_keys(struct razer_keys *keys)
{
	memset(keys->heatmap,0,sizeof(long)*22*6);
	memset(keys->rows,0,sizeof(struct razer_rgb_row)*6);
	keys->update_mask = 63;
	int i;
	for(i = 0; i < 6; ++i)
	{
		keys->rows[i].row_index = i;
	}
}
*/

struct razer_chroma_device *razer_create_device(struct razer_chroma *chroma,char *path)
{
	struct razer_chroma_device *device = (struct razer_chroma_device*)malloc(sizeof(struct razer_chroma_device));
	device->chroma = chroma;
	device->custom_mode_file = NULL;
	device->breath_mode_file = NULL;
	device->game_mode_file = NULL;
	device->none_mode_file = NULL;
	device->reactive_mode_file = NULL;
	device->spectrum_mode_file = NULL;
	device->starlight_mode_file = NULL;
	device->static_mode_file = NULL;
	device->wave_mode_file = NULL;
	device->reset_file = NULL;
	device->brightness_file = NULL;
	device->temp_clear_row_file = NULL;
	device->macro_keys_file = NULL;
	device->update_leds_file = NULL;
	device->serial_file = NULL;

	//get device name
	char *name_filename = str_CreateEmpty();
	name_filename = str_CatFree(name_filename,path);
	name_filename = str_CatFree(name_filename,razer_device_type_pathname);
	FILE *fname = fopen(name_filename,"rb");
	if(!fname)
	{
		#ifdef USE_VERBOSE_DEBUGGING
			printf("no device_type file found,skipping.\n");
		#endif
		free(device);
		free(name_filename);
		return(NULL);
	}
	fseek(fname,0,SEEK_END);
	int name_len = ftell(fname);
	rewind(fname);
	if(!name_len)
	{
		#ifdef USE_VERBOSE_DEBUGGING
			printf("device_type file empty,skipping.\n");
		#endif
		free(device);
		fclose(fname);
		free(name_filename);
		return(NULL);
	}
	//device->name = (char*)malloc(name_len+1);
	//memset(device->name,0,name_len+1);
	unsigned char buffer[4096];
	memset(&buffer,0,4096);
	int fret = fread(buffer,1,4096,fname);
	//printf("reading name with len:%d\n",name_len);
	//int fret = fread(device->name,name_len,1,fname);
	if(!fret)
	{
		#ifdef USE_VERBOSE_DEBUGGING
			printf("device name can't be read,skipping.\n");
		#endif
		free(device);
		fclose(fname);
		free(name_filename);
		return(NULL);
	}
	fclose(fname);
	free(name_filename);
	memset(&buffer[strlen((char*)buffer)-1],0,1); //removing trailing <CR>
	device->name = str_Copy((char*)&buffer);

	#ifdef USE_DEBUGGING
		printf("found device:%s\n",device->name);
	#endif

	device->rows_num = 6;
	device->columns_num = 22;
	if(!strcmp(device->name,"Razer Firefly"))
	{
		device->rows_num = 1;
		device->columns_num = 15;
	}

	//create driver attribute path+filename strings
	device->custom_mode_filename = str_CreateEmpty();
	device->custom_mode_filename = str_CatFree(device->custom_mode_filename,path);
	device->custom_mode_filename = str_CatFree(device->custom_mode_filename,razer_custom_mode_pathname);
	device->breath_mode_filename = str_CreateEmpty();
	device->breath_mode_filename = str_CatFree(device->breath_mode_filename,path);
	device->breath_mode_filename = str_CatFree(device->breath_mode_filename,razer_breath_mode_pathname);
	device->game_mode_filename = str_CreateEmpty();
	device->game_mode_filename = str_CatFree(device->game_mode_filename,path);
	device->game_mode_filename = str_CatFree(device->game_mode_filename,razer_game_mode_pathname);
	device->none_mode_filename = str_CreateEmpty();
	device->none_mode_filename = str_CatFree(device->none_mode_filename,path);
	device->none_mode_filename = str_CatFree(device->none_mode_filename,razer_none_mode_pathname);
	device->reactive_mode_filename = str_CreateEmpty();
	device->reactive_mode_filename = str_CatFree(device->reactive_mode_filename,path);
	device->reactive_mode_filename = str_CatFree(device->reactive_mode_filename,razer_reactive_mode_pathname);
	device->spectrum_mode_filename = str_CreateEmpty();
	device->spectrum_mode_filename = str_CatFree(device->spectrum_mode_filename,path);
	device->spectrum_mode_filename = str_CatFree(device->spectrum_mode_filename,razer_spectrum_mode_pathname);
	device->starlight_mode_filename = str_CreateEmpty();
	device->starlight_mode_filename = str_CatFree(device->starlight_mode_filename,path);
	device->starlight_mode_filename = str_CatFree(device->starlight_mode_filename,razer_starlight_mode_pathname);
	device->static_mode_filename = str_CreateEmpty();
	device->static_mode_filename = str_CatFree(device->static_mode_filename,path);
	device->static_mode_filename = str_CatFree(device->static_mode_filename,razer_static_mode_pathname);
	device->wave_mode_filename = str_CreateEmpty();
	device->wave_mode_filename = str_CatFree(device->wave_mode_filename,path);
	device->wave_mode_filename = str_CatFree(device->wave_mode_filename,razer_wave_mode_pathname);
	device->reset_filename = str_CreateEmpty();
	device->reset_filename = str_CatFree(device->reset_filename,path);
	device->reset_filename = str_CatFree(device->reset_filename,razer_reset_pathname);
	device->brightness_filename = str_CreateEmpty();
	device->brightness_filename = str_CatFree(device->brightness_filename,path);
	device->brightness_filename = str_CatFree(device->brightness_filename,razer_brightness_pathname);
	device->temp_clear_row_filename = str_CreateEmpty();
	device->temp_clear_row_filename = str_CatFree(device->temp_clear_row_filename,path);
	device->temp_clear_row_filename = str_CatFree(device->temp_clear_row_filename,razer_temp_clear_row_pathname);

	device->macro_keys_filename = str_CreateEmpty();
	device->macro_keys_filename = str_CatFree(device->macro_keys_filename,path);
	device->macro_keys_filename = str_CatFree(device->macro_keys_filename,razer_macro_keys_pathname);
	device->update_leds_filename = str_CreateEmpty();
	device->update_leds_filename = str_CatFree(device->update_leds_filename,path);
	device->update_leds_filename = str_CatFree(device->update_leds_filename,razer_update_leds_pathname);
	
	device->serial_filename = str_CreateEmpty();
	device->serial_filename = str_CatFree(device->serial_filename,path);
	device->serial_filename = str_CatFree(device->serial_filename,razer_serial_pathname);

	device->path = str_Copy(path);

	//device->keys = (struct razer_keys*)malloc(sizeof(struct razer_keys));
	device->leds = razer_create_rgb_frame(device->columns_num,device->rows_num);
	//razer_init_keys(device->keys);

	return(device);
}

int razer_find_devices(struct razer_chroma *chroma)
{
	struct razer_chroma_event event;
	event.type = RAZER_CHROMA_EVENT_TYPE_SYSTEM;
	event.sub_type = RAZER_CHROMA_EVENT_SUBTYPE_SYSTEM_DEVICE_ADD;
	event.key = "Device Add";
	chroma->devices = list_Create(0,0);

	DIR *d = opendir(razer_sys_hid_devices_path);
	struct dirent *entry=NULL;
	while((entry=readdir(d)))
	{
		if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..") )
			continue;
		int name_len = strlen(entry->d_name);
		char *name = (char*)malloc(name_len+1);
		memset(name,0,name_len+1);
		memcpy(name,entry->d_name,name_len);
		char *s_device_bus = strtok(name,":.");
		char *s_device_vendor_id = strtok(NULL,":.");
		char *s_device_product_id = strtok(NULL,":.");
		char *s_device_hid_id = strtok(NULL,":.");
		int device_bus,device_vendor_id,device_product_id,device_hid_id;
		sscanf(s_device_bus,"%x",&device_bus);
		sscanf(s_device_vendor_id,"%x",&device_vendor_id);
		sscanf(s_device_product_id,"%x",&device_product_id);
		sscanf(s_device_hid_id,"%x",&device_hid_id);
		if(device_vendor_id==RAZER_VENDOR_ID &&
             (device_product_id == RAZER_BLACKWIDOW_CHROMA_PRODUCT_ID || 
              device_product_id == RAZER_BLACKWIDOW_CHROMA_TE_PRODUCT_ID ||
              device_product_id == RAZER_BLACKWIDOW_OLD_PRODUCT_ID ||
              device_product_id == RAZER_BLACKWIDOW_ULTIMATE_2012_PRODUCT_ID ||
              device_product_id == RAZER_BLACKWIDOW_ULTIMATE_2013_PRODUCT_ID ||
              device_product_id == RAZER_BLACKWIDOW_ULTIMATE_2016_PRODUCT_ID ||
              device_product_id == RAZER_FIREFLY_PRODUCT_ID ||
              device_product_id == RAZER_MAMBA_CHROMA_PRODUCT_ID ||
              device_product_id == RAZER_MAMBA_CHROMA_TE_PRODUCT_ID ||
              device_product_id == RAZER_STEALTH_PRODUCT_ID))
		{
			int base_path_len = strlen(razer_sys_hid_devices_path)+strlen(entry->d_name);
			char *device_path = (char*)malloc(base_path_len+1);
			memset(device_path,0,base_path_len+1);			
			memcpy(device_path,razer_sys_hid_devices_path,strlen(razer_sys_hid_devices_path));
			memcpy(device_path+strlen(razer_sys_hid_devices_path),entry->d_name,strlen(entry->d_name));
			//#ifdef USE_VERBOSE_DEBUGGING
			#ifdef USE_VERBOSE_DEBUGGING
				printf("found possible device at path:%s\n",device_path);
			#endif
			struct razer_chroma_device *device = razer_create_device(chroma,device_path);
			free(device_path);
			if(device)
			{
				list_Push(chroma->devices,device);
				event.value = (unsigned long long)device;
				razer_fire_event(chroma,&event);
			}
		}
	}
	closedir(d);
	return(list_GetLen(chroma->devices));
}

void razer_close_device(struct razer_chroma_device *device)
{
	if(device->update_leds_file)
		razer_close_update_leds_file(device);
	if(device->custom_mode_file)
		razer_close_custom_mode_file(device);
	if(device->breath_mode_file)
		razer_close_breath_mode_file(device);
	if(device->game_mode_file)
		razer_close_game_mode_file(device);
	if(device->none_mode_file)
		razer_close_none_mode_file(device);
	if(device->reactive_mode_file)
		razer_close_reactive_mode_file(device);
	if(device->starlight_mode_file)
		razer_close_starlight_mode_file(device);
	if(device->spectrum_mode_file)
		razer_close_spectrum_mode_file(device);
	if(device->static_mode_file)
		razer_close_static_mode_file(device);
	if(device->wave_mode_file)
		razer_close_wave_mode_file(device);
	if(device->brightness_file)
		razer_close_brightness_file(device);
	if(device->temp_clear_row_file)
		razer_close_temp_clear_row_file(device);
	if(device->reset_file)
		razer_close_reset_file(device);
	if(device->macro_keys_file)
		razer_close_macro_keys_file(device);
	if(device->serial_file)
		razer_close_serial_file(device);
}

void razer_close_devices(struct razer_chroma *chroma)
{
	list_IterationReset(chroma->devices);
	while(list_IterationUnfinished(chroma->devices))
	{
		struct razer_chroma_device *device = (struct razer_chroma_device*)list_Iterate(chroma->devices);
		razer_close_device(device);
	}
}

void razer_free_device(struct razer_chroma_device *device)
{
	//free(device->keys);
	razer_free_rgb_frame(device->leds);
	free(device->path);
	free(device->name);
	if(device->custom_mode_filename)
		free(device->custom_mode_filename);
	if(device->update_leds_filename)
		free(device->update_leds_filename);
	if(device->macro_keys_filename)
		free(device->macro_keys_filename);
	if(device->breath_mode_filename)
		free(device->breath_mode_filename);
	if(device->game_mode_filename)
		free(device->game_mode_filename);
	if(device->none_mode_filename)
		free(device->none_mode_filename);
	if(device->reactive_mode_filename)
		free(device->reactive_mode_filename);
	if(device->starlight_mode_filename)
		free(device->starlight_mode_filename);
	if(device->spectrum_mode_filename)
		free(device->spectrum_mode_filename);
	if(device->static_mode_filename)
		free(device->static_mode_filename);
	if(device->wave_mode_filename)
		free(device->wave_mode_filename);
	if(device->brightness_filename)
		free(device->brightness_filename);
	if(device->reset_filename)
		free(device->reset_filename);
	if(device->temp_clear_row_filename)
		free(device->temp_clear_row_filename);
	if(device->serial_filename)
		free(device->serial_filename);
	free(device);
}

void razer_free_devices(struct razer_chroma *chroma)
{
	struct razer_chroma_device *device = NULL;
	while((device=list_Pop(chroma->devices)))
	{
		razer_free_device(device);
	}	
	list_Close(chroma->devices);
	chroma->devices = NULL;
}


struct razer_chroma_input_device *razer_create_input_device(struct razer_chroma *chroma,char *path,int type)
{
	struct razer_chroma_input_device *input_device = (struct razer_chroma_input_device*)malloc(sizeof(struct razer_chroma_input_device));
	input_device->chroma = chroma;
	input_device->input_file = 0;
	input_device->type = type;
	input_device->path = str_Copy(path);
	input_device->name = str_Copy(path); //TODO use real name here
	//get device name
	/*char *name_filename = str_CreateEmpty();
	name_filename = str_CatFree(name_filename,path);
	name_filename = str_CatFree(name_filename,razer_device_type_pathname);
	FILE *fname = fopen(name_filename,"rb");
	if(!fname)
	{
		#ifdef USE_VERBOSE_DEBUGGING
			printf("no device_type file found,skipping.\n");
		#endif
		free(device);
		free(name_filename);
		return(NULL);
	}
	fseek(fname,0,SEEK_END);
	int name_len = ftell(fname);
	rewind(fname);
	if(!name_len)
	{
		#ifdef USE_VERBOSE_DEBUGGING
			printf("device_type file empty,skipping.\n");
		#endif
		free(device);
		fclose(fname);
		free(name_filename);
		return(NULL);
	}
	//device->name = (char*)malloc(name_len+1);
	//memset(device->name,0,name_len+1);
	unsigned char buffer[4096];
	memset(&buffer,0,4096);
	int fret = fread(buffer,1,4096,fname);
	//printf("reading name with len:%d\n",name_len);
	//int fret = fread(device->name,name_len,1,fname);
	if(!fret)
	{
		#ifdef USE_VERBOSE_DEBUGGING
			printf("device name can't be read,skipping.\n");
		#endif
		free(device);
		fclose(fname);
		free(name_filename);
		return(NULL);
	}
	fclose(fname);
	free(name_filename);
	memset(&buffer[strlen((char*)buffer)-1],0,1); //removing trailing <CR>
	device->name = str_Copy((char*)&buffer);
	#ifdef USE_DEBUGGING
		printf("found device:%s\n",device->name);
	#endif
	*/
	return(input_device);
}

void razer_fire_event(struct razer_chroma *chroma,struct razer_chroma_event *event)
{
	if(chroma && event && chroma->event_handler)
		chroma->event_handler(chroma,event);
}

void razer_poll_input_device(struct razer_chroma *chroma,struct razer_chroma_input_device *input_device)
{
	struct timeval select_tv;
	struct razer_chroma_event chroma_event;
	fd_set input_rs;
	select_tv.tv_sec = 0;
	select_tv.tv_usec = 0;
	int r;
	char buf[2048];
	if(!input_device->input_file)
		razer_open_input_file(input_device);
	if(input_device->input_file)
	{
		FD_ZERO(&input_rs);
		FD_SET(input_device->input_file,&input_rs);
		r = select(input_device->input_file+1,&input_rs,0,0,&select_tv);
		if(r && FD_ISSET(input_device->input_file,&input_rs))
		{
			int n=2048;
			n = read(input_device->input_file,buf,2048);
			if(n<0)
			{
				/*if(errno != EAGAIN)
				{
					razer_close_input_file(chroma);
					return;
				}*/
			}
			else if(n==0)
			{
				razer_close_input_file(input_device);
				return;
			}
			else				
			{
				unsigned int i;
				unsigned int event_size = sizeof(struct input_event);
				for(i=0;i<n/event_size;i++)
				{
					struct input_event *event = (struct input_event*)(buf+(i*sizeof(struct input_event)));
					long diff_ms = chroma->update_ms - input_device->last_event_ms;
					input_device->event_dt = (float)diff_ms / 1000.0f;
					input_device->last_event_ms = chroma->update_ms;			

					if(input_device->type == RAZER_CHROMA_EVENT_TYPE_KEYBOARD)
					{
						chroma_event.type = RAZER_CHROMA_EVENT_TYPE_KEYBOARD;
						if(event->type==EV_KEY)
						{
							int keycode = event->code;
							if(event->value==1)
							{
								razer_copy_pos(&chroma->key_pos,&chroma->last_key_pos);
								razer_convert_keycode_to_pos(event->code,&chroma->key_pos);							
								//chroma->keys->heatmap[chroma->key_pos.y][chroma->key_pos.x]+=1;
								chroma_event.key = "Key Down";
							}
							else
								chroma_event.key = "Key Up";
							chroma_event.sub_type = event->value;
							chroma_event.value = keycode;
						}
					}
					else if (input_device->type == RAZER_CHROMA_EVENT_TYPE_MOUSE)
					{
						chroma_event.type = RAZER_CHROMA_EVENT_TYPE_MOUSE;
        				switch(event->type)
        				{
        					case EV_MSC:
        						break;
        					case EV_SYN:
        						break;
        					case EV_KEY:
        						switch(event->code)
        						{
        							case BTN_LEFT:
										chroma_event.value = RAZER_CHROMA_EVENT_BUTTON_LEFT;
										break;
	    							case BTN_MIDDLE:
										chroma_event.value = RAZER_CHROMA_EVENT_BUTTON_MIDDLE;
										break;
	    							case BTN_RIGHT:
										chroma_event.value = RAZER_CHROMA_EVENT_BUTTON_RIGHT;
										break;
	    							case BTN_EXTRA:
										chroma_event.value = RAZER_CHROMA_EVENT_BUTTON_EXTRA;
										break;
									default:
										#ifdef USE_DEBUGGING
											printf("uknown button event: type:%d,code:%d,value:%d\n",event->type,event->code,event->value);
										#endif
										break;
        						}
        						switch(event->value)
        						{
        							case 0 :
										chroma_event.sub_type = RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_BUTTON_UP;
										break;
        							case 1 :
										chroma_event.sub_type = RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_BUTTON_DOWN;
										break;
        						}
	       						break;
							case EV_REL :
								switch(event->code)
								{
									case 0 : 
										chroma_event.sub_type = RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_X_AXIS_MOVEMENT;
										chroma_event.value = event->value;
										break;
									case 1 : 
										chroma_event.sub_type = RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_Y_AXIS_MOVEMENT;
										chroma_event.value = event->value;
										break;
									case 8 : 
										chroma_event.sub_type = RAZER_CHROMA_EVENT_SUBTYPE_MOUSE_WHEEL_MOVEMENT;
										chroma_event.value = event->value;
										break;
									default:
										#ifdef USE_DEBUGGING
											printf("uknown relative movement event: type:%d,code:%d,value:%d\n",event->type,event->code,event->value);
										#endif
										break;
								}
								break;
							default:
								#ifdef USE_DEBUGGING
									printf("uknown event: type:%d,code:%d,value:%d\n",event->type,event->code,event->value);
								#endif
								break;
						}

					}
					razer_fire_event(chroma,&chroma_event);
				}
			}
		}	
	}
}

void razer_poll_input_devices(struct razer_chroma *chroma)
{
	list_IterationReset(chroma->input_devices);
	while(list_IterationUnfinished(chroma->input_devices))
	{
		struct razer_chroma_input_device *input_device = (struct razer_chroma_input_device*)list_Iterate(chroma->input_devices);
		razer_poll_input_device(chroma,input_device);
	}
}

void razer_close_input_device(struct razer_chroma_input_device *input_device)
{
	if(input_device->input_file)
		razer_close_input_file(input_device);
}

void razer_close_input_devices(struct razer_chroma *chroma)
{
	list_IterationReset(chroma->input_devices);
	while(list_IterationUnfinished(chroma->input_devices))
	{
		struct razer_chroma_input_device *input_device = (struct razer_chroma_input_device*)list_Iterate(chroma->input_devices);
		razer_close_input_device(input_device);
	}
}

void razer_free_input_device(struct razer_chroma_input_device *input_device)
{
	free(input_device->path);
	free(input_device->name);
	free(input_device);
}

void razer_free_input_devices(struct razer_chroma *chroma)
{
	struct razer_chroma_input_device *input_device = NULL;
	while((input_device=list_Pop(chroma->input_devices)))
	{
		razer_free_input_device(input_device);
	}	
	list_Close(chroma->input_devices);
	chroma->input_devices = NULL;
}

long razer_get_num_devices(struct razer_chroma *chroma)
{
	return list_GetLen(chroma->devices);
}

void razer_set_active_device_id(struct razer_chroma *chroma,int index)
{
	chroma->active_device = list_Get(chroma->devices,index);
}

void razer_set_active_device(struct razer_chroma *chroma,struct razer_chroma_device *device)
{
	chroma->active_device = device;
}

struct razer_chroma *razer_open(razer_event_handler event_handler,void *tag)
{
	struct razer_chroma *chroma =(struct razer_chroma*)malloc(sizeof(struct razer_chroma));
	chroma->event_handler = event_handler;
	chroma->tag = tag;
	#ifdef USE_DEBUGGING
		printf("opening chroma lib\n");
	#endif
	struct razer_chroma_event init;
	init.type = RAZER_CHROMA_EVENT_TYPE_SYSTEM;
	init.sub_type = RAZER_CHROMA_EVENT_SUBTYPE_SYSTEM_INIT;
	init.value = 0;
	init.key = "System Init";
	razer_fire_event(chroma,&init);
	syslog(LOG_DEBUG, "looking for chroma devices");
	if(!razer_find_devices(chroma))
	{
		razer_free_devices(chroma);
		free(chroma);
		syslog(LOG_DEBUG, "could not find devices");
		#ifdef USE_DEBUGGING
			printf("No chroma devices found!");
		#endif
		exit(1);
	}
	chroma->active_device = list_GetBottom(chroma->devices);
	chroma->active_device = list_GetTop(chroma->devices);

	syslog(LOG_DEBUG, "activating device");
	//chroma->sys_mouse_event_path = str_Copy(razer_sys_mouse_event_default_path);
	//chroma->sys_keyboard_event_path = str_Copy(razer_sys_keyboard_event_default_path);
	chroma->input_devices = list_Create(0,0);
	struct razer_chroma_input_device *keyboard = razer_create_input_device(chroma,razer_sys_keyboard_event_default_path,RAZER_CHROMA_INPUT_DEVICE_TYPE_KEYBOARD);
	struct razer_chroma_input_device *mouse = razer_create_input_device(chroma,razer_sys_mouse_event_default_path,RAZER_CHROMA_INPUT_DEVICE_TYPE_MOUSE);
	list_Push(chroma->input_devices,keyboard);
	list_Push(chroma->input_devices,mouse);

	//chroma->keyboard_input_file = 0;
	//chroma->mouse_input_file = 0;

	chroma->last_key_pos.x = -1;
	chroma->last_key_pos.y = -1;
	chroma->key_pos.x = -1;
	chroma->key_pos.y = -1;
	return(chroma);
}

void razer_close(struct razer_chroma *chroma)
{
	#ifdef USE_DEBUGGING
		printf("closing chroma lib\n");
	#endif
	syslog(LOG_DEBUG, "closing chroma library");
	chroma->event_handler = NULL;
	//free(chroma->sys_mouse_event_path);
	//free(chroma->sys_keyboard_event_path);
	//if(chroma->keyboard_input_file)
	//	razer_close_keyboard_input_file(chroma);
	//if(chroma->mouse_input_file)
	//	razer_close_mouse_input_file(chroma);
	razer_close_devices(chroma);
	razer_free_devices(chroma);
	razer_close_input_devices(chroma);
	razer_free_input_devices(chroma);
	free(chroma);
	syslog(LOG_DEBUG, "closed chroma library");
}


void razer_init_rgb_frame(struct razer_rgb_frame *frame)
{
	frame->update_mask = 0;
	int i;
	for(i = 0; i < frame->rows_num; i++)
	{
		memset(frame->rows[i]->column,0,(frame->columns_num*sizeof(struct razer_rgb)));
		frame->rows[i]->row_index = i;
		frame->rows[i]->columns_num = frame->columns_num;
		frame->update_mask |= 1<<i;
	}
}

struct razer_rgb_frame *razer_create_rgb_frame(int columns_num,int rows_num)
{
	struct razer_rgb_frame *frame = (struct razer_rgb_frame*)malloc(sizeof(struct razer_rgb_frame));
	frame->columns_num = columns_num;
	frame->rows_num = rows_num;
	struct razer_rgb_row **rows = (struct razer_rgb_row**)malloc(sizeof(struct razer_rgb_row*)*frame->rows_num);
	int i;
	for(i = 0;i < frame->rows_num; i++)
	{
		struct razer_rgb_row *row = (struct razer_rgb_row*)malloc(sizeof(struct razer_rgb_row));
		struct razer_rgb *columns = (struct razer_rgb*)malloc(sizeof(struct razer_rgb)*frame->columns_num);		
		row->column = columns;
		rows[i] = row;
	}
	frame->rows = rows;
	razer_init_rgb_frame(frame);
	return(frame);
}

void razer_free_rgb_frame(struct razer_rgb_frame *frame)
{
	//free up memory used by frame
	int i;
	for(i = 0;i < frame->rows_num; i++)
	{
		free(frame->rows[i]->column);
	}
	free(frame->rows);
	free(frame);
}

struct razer_rgb *rgb_create(unsigned char r,unsigned char g,unsigned char b)
{
	struct razer_rgb *color = (struct razer_rgb*)malloc(sizeof(struct razer_rgb));
	color->r = r;
	color->g = g;
	color->b = b;
	return(color);	
}


void razer_release_locks(struct razer_chroma_device *device)
{
	//release all key locks at once
	memset(device->locks,0,device->rows_num*device->columns_num*sizeof(int));
}

float hue2rgb(float p,float q,float t)
{
	float tt = t;
	if(tt<0.0f)
		tt+=1.0f;
	if(tt>1.0f)
		tt-=1.0f;
	if(tt < (1.0f/6.0f))
		return(p+(q-p)*6.0f*tt);
	if(tt < (1.0f/2.0f))
		return(q);
	if(tt < (2.0f/3.0f))
		return(p+(q-p)*((2.0f/3.0f)-tt)*6.0f);
	return(p);
}

void hsl2rgb(struct razer_hsl *hsl,struct razer_rgb *rgb)
{
	if(hsl->s==0.0f)
	{
		rgb->r = (unsigned char)(hsl->l*255.0f);
		rgb->g = (unsigned char)(hsl->l*255.0f);
		rgb->b = (unsigned char)(hsl->l*255.0f);
		return;
	}
	float q,p;
	if(hsl->l < 0.5f)
		q = hsl->l*(1.0f+hsl->s);
	else
		q = hsl->l + hsl->s - hsl->l * hsl->s;
	p = 2.0f * hsl->l - q;
	rgb->r = (unsigned char)(hue2rgb(p,q,hsl->h+(1.0f/3.0f))*255.0f);
	rgb->g = (unsigned char)(hue2rgb(p,q,hsl->h)*255.0f);
	rgb->b = (unsigned char)(hue2rgb(p,q,hsl->h-(1.0f/3.0f))*255.0f);
}

void rgb_from_hue(float percentage,float start_hue,float end_hue,struct razer_rgb *color)
{
	struct razer_hsl hsl;
	hsl.h=percentage*(end_hue-start_hue)+start_hue;
	hsl.l=0.5f;
	hsl.s=1.0f;
	hsl2rgb(&hsl,color);
}

unsigned char rgb_clamp(int v)
{
	if(v>255)
		return(255);
	if(v<0)
		return(0);
	return((unsigned char)v);
}

void rgb_add(struct razer_rgb *dst,struct razer_rgb *src)
{
	dst->r = rgb_clamp(dst->r + src->r);
	dst->g = rgb_clamp(dst->g + src->g);
	dst->b = rgb_clamp(dst->b + src->b);
}

void rgb_mix(struct razer_rgb *dst,struct razer_rgb *src,float dst_opacity)
{
	dst->r = rgb_clamp((1.0f-dst_opacity)*dst->r + src->r*dst_opacity);
	dst->g = rgb_clamp((1.0f-dst_opacity)*dst->g + src->g*dst_opacity);
	dst->b = rgb_clamp((1.0f-dst_opacity)*dst->b + src->b*dst_opacity);
}

void rgb_mix_into(struct razer_rgb *dst,struct razer_rgb *src_a,struct razer_rgb *src_b,float dst_opacity)
{
	dst->r = rgb_clamp((1.0f-dst_opacity)*src_a->r + src_b->r*dst_opacity);
	dst->g = rgb_clamp((1.0f-dst_opacity)*src_a->g + src_b->g*dst_opacity);
	dst->b = rgb_clamp((1.0f-dst_opacity)*src_a->b + src_b->b*dst_opacity);
}

struct razer_rgb *rgb_copy(struct razer_rgb *color)
{
	struct razer_rgb *copy = (struct razer_rgb*)malloc(sizeof(struct razer_rgb));
	copy->r = color->r;
	copy->g = color->g;
	copy->b = color->b;
	return(copy);
}

struct razer_pos *razer_pos_copy(struct razer_pos *pos)
{
	struct razer_pos *copy = (struct razer_pos*)malloc(sizeof(struct razer_pos));
	copy->x = pos->x;
	copy->y = pos->y;
	return(copy);
}


void razer_convert_keycode_to_pos(int keycode,struct razer_pos *pos)
{
	switch(keycode)
	{
		case 1:/*ESC*/
			pos->x=1;
			pos->y=0;
		break;
		case 59:/*F1-F10*/
		case 60:
		case 61:
		case 62:/*buggy*/
		case 63:
		case 64:
		case 65:
		case 66:
		case 67:
		case 68:
			pos->x=3+(keycode-59);
			pos->y=0;
		break;
		case 87:/*F11*/
			pos->x=13;
			pos->y=0;
		break;
		case 88:/*F12*/
			pos->x=14;
			pos->y=0;
		break;
		case 99:/*printscreen*/
			pos->x=15;
			pos->y=0;
		break;
		case 70:/*roll*/
			pos->x=16;
			pos->y=0;
		break;
		case 119:/*pause/sys req*/
			pos->x=17;
			pos->y=0;
		break;
		case 183:/*M1*/
			pos->x=0;
			pos->y=1;
		break;
		case 41:/*caret*/
			pos->x=1;
			pos->y=1;
		break;
		case 2:/*1 - 10*/
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:/*question mark*/
		case 13:/*quotes*/
		case 14:/*backspace*/
			pos->x=2 +(keycode-2);
			pos->y=1;
		break;
		case 110:/*insert*/
			pos->x=15;
			pos->y=1;
		break;
		case 102:/*home*/
			pos->x=16;
			pos->y=1;
		break;
		case 104:/*pgup*/
			pos->x=17;
			pos->y=1;
		break;
		case 69:/*numlock*/
			pos->x=18;
			pos->y=1;
		break;
		case 98:/*numpad divide*/
			pos->x=19;
			pos->y=1;
		break;
		case 55:/*numpad multiply*/
			pos->x=20;
			pos->y=1;
		break;
		case 74:/*numpad subtract*/
			pos->x=21;
			pos->y=1;
		break;
		case 184:/*M2*/
			pos->x=0;
			pos->y=2;
		break;
		case 15:/*tabulator*/
			pos->x=1;
			pos->y=2;
		break;
		case 16:/*q-asterisk*/
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:/*asterisk*/
			pos->x=2+(keycode-16);
			pos->y=2;
		break;
		case 111:/*delete*/
			pos->x=15;
			pos->y=2;
		break;
		case 107:/*end*/
			pos->x=16;
			pos->y=2;
		break;
		case 109:/*pgdown*/
			pos->x=17;
			pos->y=2;
		break;
		case 71:/*numpad 7*/
			pos->x=18;
			pos->y=2;
		break;
		case 72:/*numpad 8*/
			pos->x=19;
			pos->y=2;
		break;
		case 73:/*numpad 9*/
			pos->x=20;
			pos->y=2;
		break;
		case 78:/*numpad add*/
			pos->x=21;
			pos->y=2;
		break;
		case 185:/*M3*/
			pos->x=0;
			pos->y=3;
		break;
		case 58:/*capslock*/
			pos->x=1;
			pos->y=3;
		break;
		case 43:/*grave*/
			pos->x=13;
			pos->y=3;
		break;
		case 28:/*return*/
			pos->x=14;
			pos->y=3;
		break;
		case 30:/*a-grave*/
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 280:
			pos->x=2+(keycode-30);
			pos->y=3;
		break;
		case 75:/*numpad 4*/
			pos->x=18;
			pos->y=3;
		break;
		case 76:/*numpad 5*/
			pos->x=19;
			pos->y=3;
		break;
		case 77:/*numapd 6*/
			pos->x=20;
			pos->y=3;
		break;
		case 186:/*M4*/
			pos->x=0;
			pos->y=4;
		break;
		case 42:/*left shift*/
			pos->x=1;
			pos->y=4;
		break;
		case 86:/*arrows*/
			pos->x=2;
			pos->y=4;
		break;
		case 44:/*y-right shift*/
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
			pos->x=3+(keycode-44);
			pos->y=4;
		break;
		case 54:/*right shift*/
			pos->x=14;
			pos->y=4;
		break;
		case 103:/*cursor up*/
			pos->x=16;
			pos->y=4;
		break;
		case 79:/*numpad 1*/
			pos->x=18;
			pos->y=4;
		break;
		case 80:/*numpad 2*/
			pos->x=19;
			pos->y=4;
		break;
		case 81:/*numpad 3*/
			pos->x=20;
			pos->y=4;
		break;
		case 96:/*numpad enter*/
			pos->x=21;
			pos->y=4;
		break;
		case 187:/*M5*/
			pos->x=0;
			pos->y=5;
		break;
		case 29:/*left control*/
			pos->x=1;
			pos->y=5;
		break;
		case 125:/*left windows*/
			pos->x=2;
			pos->y=5;
		break;
		case 56:/*left alt*/
			pos->x=3;
			pos->y=5;
		break;
		case 100:/*right alt*/
			pos->x=11;
			pos->y=5;
		break;
		case 194:/*FN*/
			pos->x=12;
			pos->y=5;
		break;
		case 127:/*window context*/
			pos->x=13;
			pos->y=5;
		break;
		case 97:/*right control*/
			pos->x=14;
			pos->y=5;
		break;
		case 105:/*cursor left*/
			pos->x=15;
			pos->y=5;
		break;
		case 108:/*cursor down*/
			pos->x=16;
			pos->y=5;
		break;
		case 106:/*cursor right*/
			pos->x=17;
			pos->y=5;
		break;
		case 82:/*numpad insert*/
			pos->x=19;
			pos->y=5;
		break;
		case 83:/*numpad delete*/
			pos->x=20;
			pos->y=5;
		break;
		case 57:/**/
			pos->x=7;
			pos->y=5;
		break;
		default:
			#ifdef USE_DEBUGGING
				printf("unknown key:%d\n",keycode);
			#endif
		break;
	}
}

/*void razer_convert_pos_to_keycode(struct razer_pos *pos,int *keycode)
{

}*/

int razer_get_key_class(int keycode)
{
	switch(keycode)
	{
		case 59:/*F1-F10*/
		case 60:
		case 61:
		case 62:
		case 63:
		case 64:
		case 65:
		case 66:
		case 67:
		case 68:
		case 87:/*F11*/
		case 88:/*F12*/
			return(RAZER_KEY_CLASS_FUNCTION_KEYS);
		case 183:/*M1*/
		case 184:/*M2*/
		case 185:/*M3*/
		case 186:/*M4*/
		case 187:/*M5*/
			return(RAZER_KEY_CLASS_MACRO_KEYS);
		case 99:/*printscreen*/
		case 70:/*roll*/
		case 119:/*pause/sys req*/
			return(RAZER_KEY_CLASS_SYSTEM_CONTROLS);
		case 41:/*caret*/
		case 2:/*1 - 10*/
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			return(RAZER_KEY_CLASS_NUMBERS);
		case 110:/*insert*/
		case 111:/*delete*/
		case 102:/*home*/
		case 107:/*end*/
		case 104:/*pgup*/
		case 109:/*pgdown*/
			return(RAZER_KEY_CLASS_POSITION_CONTROLS);
		case 98:/*numpad divide*/
		case 55:/*numpad multiply*/
		case 74:/*numpad subtract*/
		case 78:/*numpad add*/
			return(RAZER_KEY_CLASS_NUMPAD_OPERATIONS);
		case 79:/*numpad 1*/
		case 80:/*numpad 2*/
		case 81:/*numpad 3*/
		case 75:/*numpad 4*/
		case 76:/*numpad 5*/
		case 77:/*numapd 6*/
		case 71:/*numpad 7*/
		case 72:/*numpad 8*/
		case 73:/*numpad 9*/
		case 82:/*numpad insert*/
		case 83:/*numpad delete*/
			return(RAZER_KEY_CLASS_NUMPAD_NUMBERS);
		case 96:/*numpad enter*/
		case 69:/*numlock*/
			return(RAZER_KEY_CLASS_NUMPAD_CONTROLS);
		case 1:/*ESC*/
		case 15:/*tabulator*/
		case 58:/*capslock*/
		case 42:/*left shift*/
		case 56:/*left alt*/
		case 29:/*left control*/
		case 125:/*left windows*/
			return(RAZER_KEY_CLASS_LEFT_CONTROLS);
		case 14:/*backspace*/
		case 28:/*return*/
		case 54:/*right shift*/
		case 100:/*right alt*/
		case 194:/*FN*/
		case 127:/*window context*/
		case 97:/*right control*/
			return(RAZER_KEY_CLASS_RIGHT_CONTROLS);
		case 103:/*cursor up*/
		case 105:/*cursor left*/
		case 108:/*cursor down*/
		case 106:/*cursor right*/
			return(RAZER_KEY_CLASS_ARROWS);
		case 12:/*question mark*/
		case 13:/*quotes*/
		case 16:/*q-asterisk*/
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:/*asterisk*/
		case 43:/*grave*/
		case 30:/*a-grave*/
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 280:
		case 86:/*arrows*/
		case 44:/*y-right shift*/
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
			return(RAZER_KEY_CLASS_LETTERS);
		default:
			#ifdef USE_DEBUGGING
				printf("unknown key:%d\n",keycode);
			#endif
			return(RAZER_KEY_CLASS_UNKNOWN);
	}
}

void razer_convert_ascii_to_pos(unsigned char letter,struct razer_pos *pos)
{
	switch(letter)
	{
		case 27:/*ESC*/
			pos->x=1;
			pos->y=0;
		break;
		/*case 59://F1-F10
		case 60:
		case 61:
		case 62:
		case 63:
		case 64:
		case 65:
		case 66:
		case 67:
		case 68:
			pos->x=3+(letter-59);
			pos->y=0;
		break;*/
		/*case 87://F11
			pos->x=13;
			pos->y=0;
		break;*/
		/*case 88://F12
			pos->x=14;
			pos->y=0;
		break;*/
		/*case 99://printscreen
			pos->x=15;
			pos->y=0;
		break;*/
		/*case 70://roll
			pos->x=16;
			pos->y=0;
		break;*/
		/*case 119://pause/sys req
			pos->x=17;
			pos->y=0;
		break;*/
		/*case 183://M1
			pos->x=0;
			pos->y=1;
		break;*/
		case 94:/*caret*/
		case 186:
			pos->x=1;
			pos->y=1;
		break;
		case 49:/*1*/
		case 33:
			pos->x=2;
			pos->y=1;
		break;
		case 50:/*2*/
		case 34:
		case 178:
			pos->x=3;
			pos->y=1;
		break;
		case 51:/*3*/
		case 167:
		case 179:
			pos->x=4;
			pos->y=1;
		break;
		case 52:/*4*/
		case 36:
			pos->x=5;
			pos->y=1;
		break;
		case 53:/*5*/
		case 37:
			pos->x=6;
			pos->y=1;
		break;
		case 54:/*6*/
		case 38:
			pos->x=7;
			pos->y=1;
		break;
		case 55:/*7*/
		case 47:
		case 123:
			pos->x=8;
			pos->y=1;
		break;
		case 56:/*8*/
		case 40:
		case 91:
			pos->x=9;
			pos->y=1;
		break;
		case 57:/*9*/
		case 41:
		case 93:
			pos->x=10;
			pos->y=1;
		break;
		case 48:/*0*/
		case 61:
		case 125:
			pos->x=11;
			pos->y=1;
		break;
		case 63:/*question mark*/
		case 92:
			pos->x=12;
			pos->y=1;
		break;
		case 96:/*quotes*/
			pos->x=13;
			pos->y=1;
		break;
		case 8:/*backspace*/
			pos->x=14;
			pos->y=1;
		break;
		/*case 110://insert
			pos->x=15;
			pos->y=1;
		break;
		case 102://home
			pos->x=16;
			pos->y=1;
		break;
		case 104://pgup
			pos->x=17;
			pos->y=1;
		break;
		case 69://numlock
			pos->x=18;
			pos->y=1;
		break;
		case 98://numpad divide
			pos->x=19;
			pos->y=1;
		break;
		case 55://numpad multiply
			pos->x=20;
			pos->y=1;
		break;
		case 74://numpad subtract
			pos->x=21;
			pos->y=1;
		break;
		case 184://M2
			pos->x=0;
			pos->y=2;
		break;*/
		case 9:/*tabulator*/
			pos->x=1;
			pos->y=2;
		break;
		case 113:/*q*/
		case 81:
			pos->x=2;
			pos->y=2;
		break;
		case 119:/*w*/
		case 87:
			pos->x=3;
			pos->y=2;
		break;
		case 101:/*e*/
		case 69:
		case 128:
			pos->x=4;
			pos->y=2;
		break;
		case 114:/*r*/
		case 82:
			pos->x=5;
			pos->y=2;
		break;
		case 116:/*t*/
		case 84:
			pos->x=6;
			pos->y=2;
		break;
		case 122:/*z*/
		case 90:
			pos->x=7;
			pos->y=2;
		break;
		case 117:/*u*/
		case 85:
			pos->x=8;
			pos->y=2;
		break;
		case 105:/*i*/
		case 73:
			pos->x=9;
			pos->y=2;
		break;
		case 111:/*o*/
		case 79:
			pos->x=10;
			pos->y=2;
		break;
		case 112:/*p*/
		case 80:
			pos->x=11;
			pos->y=2;
		break;
		case 252:/*ue*/
		case 220:
			pos->x=12;
			pos->y=2;
		break;
		case 42:/*asterisk*/
		case 43:
		case 152:
			pos->x=13;
			pos->y=2;
		break;
		case 127:/*delete*/
			pos->x=15;
			pos->y=2;
		break;
		/*case 107://end
			pos->x=16;
			pos->y=2;
		break;
		case 109://pgdown
			pos->x=17;
			pos->y=2;
		break;
		case 71://numpad 7
			pos->x=18;
			pos->y=2;
		break;
		case 72://numpad 8
			pos->x=19;
			pos->y=2;
		break;
		case 73://numpad 9
			pos->x=20;
			pos->y=2;
		break;
		case 78://numpad add
			pos->x=21;
			pos->y=2;
		break;
		case 185://M3
			pos->x=0;
			pos->y=3;
		break;
		case 58://capslock
			pos->x=1;
			pos->y=3;
		break;*/
		case 97:/*a*/
		case 65:
		case 64:
			pos->x=2;
			pos->y=3;
		break;
		case 115:/*s*/
		case 83:
			pos->x=3;
			pos->y=3;
		break;
		case 100:/*d*/
		case 68:
			pos->x=4;
			pos->y=3;
		break;
		case 102:/*f*/
		case 70:
			pos->x=5;
			pos->y=3;
		break;
		case 103:/*g*/
		case 71:
			pos->x=6;
			pos->y=3;
		break;
		case 104:/*h*/
		case 72:
			pos->x=7;
			pos->y=3;
		break;
		case 106:/*j*/
		case 74:
			pos->x=8;
			pos->y=3;
		break;
		case 107:/*k*/
		case 75:
			pos->x=9;
			pos->y=3;
		break;
		case 108:/*l*/
		case 76:
			pos->x=10;
			pos->y=3;
		break;
		case 246:/*oe*/
		case 214:
			pos->x=11;
			pos->y=3;
		break;
		case 228:/*ae*/
		case 196:
			pos->x=12;
			pos->y=3;
		break;
		case 35:/*grave*/
		case 39:
			pos->x=13;
			pos->y=3;
		break;
		case 13:/*return*/
			pos->x=14;
			pos->y=3;
		break;
		/*case 75://numpad 4
			pos->x=18;
			pos->y=3;
		break;
		case 76://numpad 5
			pos->x=19;
			pos->y=3;
		break;
		case 77://numapd 6
			pos->x=20;
			pos->y=3;
		break;
		case 186://M4
			pos->x=0;
			pos->y=4;
		break;
		case 42://left shift
			pos->x=1;
			pos->y=4;
		break;*/
		case 60:/*arrows*/
		case 62:
		case 124:
			pos->x=2;
			pos->y=4;
		break;
		case 121:/*y*/
		case 89:
			pos->x=3;
			pos->y=4;
		break;
		case 120:/*x*/
		case 88:
			pos->x=4;
			pos->y=4;
		break;
		case 99:/*c*/
		case 67:
			pos->x=5;
			pos->y=4;
		break;
		case 118:/*v*/
		case 86:
			pos->x=6;
			pos->y=4;
		break;
		case 98:/*b*/
		case 66:
			pos->x=7;
			pos->y=4;
		break;
		case 110:/*n*/
		case 78:
			pos->x=8;
			pos->y=4;
		break;
		case 109:/*m*/
		case 77:
			pos->x=9;
			pos->y=4;
		break;
		case 44:/*,*/
		case 59:
			pos->x=10;
			pos->y=4;
		break;
		case 46:/*.*/
		case 58:
			pos->x=11;
			pos->y=4;
		break;
		case 45:/*-*/
		case 95:
			pos->x=12;
			pos->y=4;
		break;
		/*case 54://right shift
			pos->x=14;
			pos->y=4;
		break;
		case 103://cursor up
			pos->x=16;
			pos->y=4;
		break;
		case 79://numpad 1
			pos->x=18;
			pos->y=4;
		break;
		case 80://numpad 2
			pos->x=19;
			pos->y=4;
		break;
		case 81://numpad 3
			pos->x=20;
			pos->y=4;
		break;
		case 96://numpad enter
			pos->x=21;
			pos->y=4;
		break;
		case 187://M5
			pos->x=0;
			pos->y=5;
		break;
		case 29://left control
			pos->x=1;
			pos->y=5;
		break;
		case 125://left windows
			pos->x=2;
			pos->y=5;
		break;
		case 56://left alt
			pos->x=3;
			pos->y=5;
		break;
		case 100://right alt
			pos->x=11;
			pos->y=5;
		break;
		case 194://FN
			pos->x=12;
			pos->y=5;
		break;
		case 127://window context
			pos->x=13;
			pos->y=5;
		break;
		case 97://right control
			pos->x=14;
			pos->y=5;
		break;
		case 105://cursor left
			pos->x=15;
			pos->y=5;
		break;
		case 108://cursor down
			pos->x=16;
			pos->y=5;
		break;
		case 106://cursor right
			pos->x=17;
			pos->y=5;
		break;
		case 82://numpad insert
			pos->x=19;
			pos->y=5;
		break;
		case 83://numpad delete
			pos->x=20;
			pos->y=5;
		break;*/
		default:
			#ifdef USE_DEBUGGING
				printf("no known key for ascii:%d\n",letter);
			#endif
		break;
	}
}

int razer_device_enable_macro_keys(struct razer_chroma_device *device) //TODO add enable parameter
{
	if(!device->macro_keys_file)
		razer_open_macro_keys_file(device);
	if(device->macro_keys_file)
	{
		if(fwrite("1",1,1,device->macro_keys_file))
		{
			fflush(device->macro_keys_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error enabling macro keys\n");
	#endif
	return(0);	
}

int razer_enable_macro_keys(struct razer_chroma *chroma)
{
	//enabling macro keys of active device
	return(razer_device_enable_macro_keys(chroma->active_device));
}

int razer_device_get_serial(struct razer_chroma_device *device, char* buffer)
{
	read_from_device_file(device->serial_filename, buffer, 15);
	return(0);
}

int razer_device_get_name(struct razer_chroma_device *device, char* buffer)
{
	if(device->name != NULL)
	{
		strcpy(buffer, device->name);
	}
	return(0);
}


int razer_get_serial(struct razer_chroma *chroma, char* buffer)
{
	//getting device serial number
	return(razer_device_get_serial(chroma->active_device, buffer));
}

int razer_get_name(struct razer_chroma *chroma, char* buffer)
{
	//getting device name / type
	return(razer_device_get_name(chroma->active_device, buffer));
}

int razer_device_set_custom_mode(struct razer_chroma_device *device)
{
	if(!device->custom_mode_file)
		razer_open_custom_mode_file(device);
	if(device->custom_mode_file)
	{
		if(fwrite("1",1,1,device->custom_mode_file))
		{
			fflush(device->custom_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to custom\n");
	#endif
	return(0);	
}

int razer_set_custom_mode(struct razer_chroma *chroma)
{
	//setting active device to custom mode
	return(razer_device_set_custom_mode(chroma->active_device));
}

int razer_device_set_breath_mode(struct razer_chroma_device *device,struct razer_rgb *first_color,struct razer_rgb *second_color)
{
	if(!device->breath_mode_file)
		razer_open_breath_mode_file(device);
	if(device->breath_mode_file)
	{
		if(!fwrite(first_color,sizeof(struct razer_rgb),1,device->breath_mode_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing first breath mode color\n");
			#endif
			return(0);
		}
		if(!fwrite(second_color,sizeof(struct razer_rgb),1,device->breath_mode_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing second breath mode color\n");
			#endif
			return(0);
		}
		fflush(device->breath_mode_file);
		return(1);
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to breath\n");
	#endif
	return(0);	
}

int razer_set_breath_mode(struct razer_chroma *chroma,struct razer_rgb *first_color,struct razer_rgb *second_color)
{
	//setting active device to breath mode
	return(razer_device_set_breath_mode(chroma->active_device,first_color,second_color));
}

int razer_device_set_one_color_breath_mode(struct razer_chroma_device *device,struct razer_rgb *first_color)
{
	if(!device->breath_mode_file)
		razer_open_breath_mode_file(device);
	if(device->breath_mode_file)
	{
		if(!fwrite(first_color,sizeof(struct razer_rgb),1,device->breath_mode_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing breath mode color\n");
			#endif
			return(0);
		}
		fflush(device->breath_mode_file);
		return(1);
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to breath\n");
	#endif
	return(0);	
}

int razer_set_one_color_breath_mode(struct razer_chroma *chroma,struct razer_rgb *first_color)
{
	//setting active device to breath mode
	return(razer_device_set_one_color_breath_mode(chroma->active_device,first_color));
}

int razer_device_set_random_breath_mode(struct razer_chroma_device *device)
{
	if(!device->breath_mode_file)
		razer_open_breath_mode_file(device);
	if(device->breath_mode_file)
	{
		if(fwrite("1",1,1,device->breath_mode_file))
		{
			fflush(device->breath_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to random breath\n");
	#endif
	return(0);	
}

int razer_set_random_breath_mode(struct razer_chroma *chroma)
{
	//setting active device to random breath mode
	return(razer_device_set_random_breath_mode(chroma->active_device));
}

int razer_device_set_game_mode(struct razer_chroma_device *device,unsigned char enable)
{
	if(!device->game_mode_file)
		razer_open_game_mode_file(device);
	if(device->game_mode_file)
	{
		char buf[32];
		sprintf(buf, "%d", enable);
		if(fwrite(buf,strlen(buf),1,device->game_mode_file))
		{
			fflush(device->game_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to game\n");
	#endif
	return(0);	
}

int razer_set_game_mode(struct razer_chroma *chroma,unsigned char enable)
{
	//setting active device to game mode
	return(razer_device_set_game_mode(chroma->active_device,enable));
}

int razer_device_set_none_mode(struct razer_chroma_device *device)
{
	if(!device->none_mode_file)
		razer_open_none_mode_file(device);
	if(device->none_mode_file)
	{
		if(fwrite("1",1,1,device->none_mode_file))
		{
			fflush(device->none_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to none\n");
	#endif
	return(0);	
}

int razer_set_none_mode(struct razer_chroma *chroma)
{
	//setting active device to none mode
	return(razer_device_set_none_mode(chroma->active_device));
}

int razer_device_set_reactive_mode(struct razer_chroma_device *device,unsigned char speed,struct razer_rgb *color)
{
	if(!device->reactive_mode_file)
		razer_open_reactive_mode_file(device);
	if(device->reactive_mode_file)
	{
		if(!fwrite(&speed,1,1,device->reactive_mode_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing speed to reactive\n");
			#endif
			return(0);
		}
		if(!fwrite(color,sizeof(struct razer_rgb),1,device->reactive_mode_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing speed to reactive\n");
			#endif
			return(0);
		}
		fflush(device->reactive_mode_file);
		return(1);
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to reactive\n");
	#endif
	return(0);	
}

int razer_set_reactive_mode(struct razer_chroma *chroma,unsigned char speed,struct razer_rgb *color)
{
	//setting active device to reactive mode
	return(razer_device_set_reactive_mode(chroma->active_device,speed,color));
}

int razer_device_set_spectrum_mode(struct razer_chroma_device *device)
{
	if(!device->spectrum_mode_file)
		razer_open_spectrum_mode_file(device);
	if(device->spectrum_mode_file)
	{
		if(fwrite("1",1,1,device->spectrum_mode_file))
		{
			fflush(device->spectrum_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to spectrum\n");
	#endif
	return(0);	
}

int razer_set_spectrum_mode(struct razer_chroma *chroma)
{
	//setting active device to spectrum mode
	return(razer_device_set_spectrum_mode(chroma->active_device));
}

int razer_device_set_static_mode(struct razer_chroma_device *device,struct razer_rgb *color)
{
	if(!device->static_mode_file)
		razer_open_static_mode_file(device);
	if(device->static_mode_file)
	{
		if(fwrite(color,sizeof(struct razer_rgb),1,device->static_mode_file))
		{
			fflush(device->static_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to static\n");
	#endif
	return(0);	
}

int razer_set_static_mode(struct razer_chroma *chroma,struct razer_rgb *color)
{
	//setting active device to static mode
	return(razer_device_set_static_mode(chroma->active_device,color));
}

int razer_device_set_wave_mode(struct razer_chroma_device *device,unsigned char direction)
{
	if(!device->wave_mode_file)
		razer_open_wave_mode_file(device);
	if(device->wave_mode_file)
	{
		char buf[32];
		sprintf(buf, "%d", direction);
		if(fwrite(buf,strlen(buf),1,device->wave_mode_file))
		{
			fflush(device->wave_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to wave\n");
	#endif
	return(0);	
}

int razer_set_wave_mode(struct razer_chroma *chroma,unsigned char direction)
{
	//setting active device to wave mode
	return(razer_device_set_wave_mode(chroma->active_device,direction));
}

int razer_device_set_starlight_mode(struct razer_chroma_device *device)
{
	if(!device->starlight_mode_file)
		razer_open_starlight_mode_file(device);
	if(device->starlight_mode_file)
	{
		if(fwrite("1",1,1,device->wave_mode_file))
		{
			fflush(device->wave_mode_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to starlight\n");
	#endif
	return(0);	
}

int razer_set_starlight_mode(struct razer_chroma *chroma)
{
	//setting active device to wave mode
	return(razer_device_set_starlight_mode(chroma->active_device));
}

int razer_device_reset(struct razer_chroma_device *device)
{
	if(!device->reset_file)
		razer_open_reset_file(device);
	if(device->reset_file)
	{
		if(fwrite("1",1,1,device->reset_file))
		{
			fflush(device->reset_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error resetting\n");
	#endif
	return(0);	
}

int razer_reset_mode(struct razer_chroma *chroma)
{
	//resetting active device
	return(razer_device_reset(chroma->active_device));
}

int razer_device_set_brightness(struct razer_chroma_device *device,unsigned char brightness)
{
	if(!device->brightness_file)
		razer_open_brightness_file(device);
	if(device->brightness_file)
	{
		char buf[32];
		sprintf(buf, "%d", brightness);
		if(fwrite(buf,strlen(buf),1,device->brightness_file))
		{
			fflush(device->brightness_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting brighness\n");
	#endif
	return(0);	
}

int razer_set_brightness(struct razer_chroma *chroma,unsigned char brightness)
{
	//setting active device brightness
	return(razer_device_set_brightness(chroma->active_device,brightness));
}

int razer_device_temp_clear_row(struct razer_chroma_device *device)
{
	if(!device->temp_clear_row_file)
		razer_open_temp_clear_row_file(device);
	if(device->temp_clear_row_file)
	{
		if(fwrite("1",1,1,device->temp_clear_row_file))
		{
			fflush(device->temp_clear_row_file);
			return(1);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error temporarily clearing row\n");
	#endif
	return(0);	
}

int razer_temp_clear_row(struct razer_chroma *chroma)
{
	//temporarily clearing row of active device
	return(razer_device_temp_clear_row(chroma->active_device));
}

int razer_device_set_led_row(struct razer_chroma_device *device,unsigned char row_index,unsigned char num_colors,struct razer_rgb *colors)
{
	if(!device->update_leds_file)
		razer_open_update_leds_file(device);
	if(device->update_leds_file)
	{
		if(!fwrite(&row_index,1,1,device->update_leds_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing led row index\n");
			#endif
		}
		if(!fwrite(colors,num_colors*3,1,device->update_leds_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing led row colors:%d\n",num_colors);
			#endif
		}
		fflush(device->update_leds_file);
		return(1);
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting led row\n");
	#endif
	return(0);	
}

int razer_set_led_row(struct razer_chroma *chroma,unsigned char row_index,unsigned char num_colors,struct razer_rgb *colors)
{
	//setting led row of active device
	return(razer_device_set_led_row(chroma->active_device,row_index,num_colors,colors));
}

int razer_device_set_led_row_buffered(struct razer_chroma_device *device,unsigned char *buffer,int buffer_len)
{
	if(!device->update_leds_file)
		razer_open_update_leds_file(device);
	if(device->update_leds_file)
	{
		if(!fwrite(buffer,buffer_len,1,device->update_leds_file))
		{
			#ifdef USE_DEBUGGING
				printf("error writing led row buffered colors; buffer_len:%d\n",buffer_len);
			#endif
		}
		fflush(device->update_leds_file);
		return(1);
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting led row\n");
	#endif
	return(0);	
}

int razer_set_led_row_buffered(struct razer_chroma *chroma,unsigned char *buffer,int buffer_len)
{
	//setting led row of active device
	return(razer_device_set_led_row_buffered(chroma->active_device,buffer,buffer_len));
}




/*
int razer_device_update_keys(struct razer_chroma_device *device, struct razer_keys *keys)
{
	int i;
	if(!keys->update_mask)
		return(1);
	if(!device->update_keys_file)
		razer_open_update_keys_file(device);
	if(device->update_keys_file)
	{
		for(i=0;i<6;i++)
		{
			if(keys->update_mask &(1<<i))
				if(!fwrite((void*)&keys->rows[i],sizeof(struct razer_rgb_row),1,device->update_keys_file))
				{
					#ifdef USE_DEBUGGING
						printf("error writing to set_key_row attribute\n");
					#endif
					return(0);
				}
		}
	}
	else
	{
		#ifdef USE_DEBUGGING
			printf("error updating keys\n");
		#endif
		return(0);
	}
	fflush(device->update_keys_file);
    keys->update_mask=0;
   	return(razer_device_set_custom_mode(device));
}

int razer_update_keys(struct razer_chroma *chroma, struct razer_keys *keys)
{
	//updating keys of active device	
	return(razer_device_update_keys(chroma->active_device,keys));
}
*/

int razer_device_update_leds(struct razer_chroma_device *device, struct razer_rgb_frame *frame)
{
	int i;
	if(!frame->update_mask)
		return(1);
	if(!device->update_leds_file)
		razer_open_update_leds_file(device);
	if(device->update_leds_file)
	{
		for(i=0;i<device->rows_num;i++)
		{
			if(frame->update_mask &(1<<i))
				//if(!fwrite((void*)&frame->rows[i],sizeof(struct razer_rgb_row),1,device->update_leds_file))
				/*if(!fwrite((void*)&frame->rows[i],(sizeof(struct razer_rgb)*frame->columns_num)+1,1,device->update_leds_file))
				{
					#ifdef USE_DEBUGGING
						printf("error writing to set_key_row attribute\n");
					#endif
					return(0);
				}*/
				razer_device_set_led_row(device,frame->rows[i]->row_index,frame->rows[i]->columns_num,frame->rows[i]->column);
		}
	}
	else
	{
		#ifdef USE_DEBUGGING
			printf("error updating frame\n");
		#endif
			return(0);
	}
	fflush(device->update_leds_file);
    frame->update_mask=0;
   	return(razer_device_set_custom_mode(device));
}

int razer_update_leds(struct razer_chroma *chroma, struct razer_rgb_frame *frame)
{
	//updating frame of active device	
	return(razer_device_update_leds(chroma->active_device,frame));
}

void razer_clear_frame(struct razer_rgb_frame *frame)
{
	int i;
	for(i=0;i<frame->rows_num;i++)
		memset((void*)((char*)frame->rows[i]->column+1),0,(sizeof(struct razer_rgb)*frame->columns_num));
	frame->update_mask = 0;
}

void razer_copy_row(struct razer_rgb_row *src_row,struct razer_rgb_row *dst_row)
{
	int len = dst_row->columns_num;
	if(src_row->columns_num<len)
		len = src_row->columns_num;
	memcpy(dst_row->column,src_row->column,len*sizeof(struct razer_rgb));
}

//TODO can and should be optimized
//no use_update_mask or update_mask filled
//and same dimensions 
//just copy the whole rows member
//void razer_copy_rows(struct razer_rgb_row *src_rows,struct razer_rgb_row *dst_rows,int update_mask,int use_update_mask)
void razer_copy_frame_rows(struct razer_rgb_frame *src_frame,struct razer_rgb_frame *dst_frame,int use_update_mask)
{
	int i;
	int len = dst_frame->rows_num;
	if(src_frame->rows_num<len)
		len = src_frame->rows_num;
	for(i=0;i<len;i++)
	{
		if(use_update_mask)
			if(src_frame->update_mask &(1<<i))
			{
				//memcpy((void*)((char*)dst_rows+(i*sizeof(struct razer_rgb_row))),(void*)((char*)src_rows+(i*sizeof(struct razer_rgb_row))),sizeof(struct razer_rgb_row));
				//memcpy((void*)((char*)dst_rows+(i*sizeof(struct razer_rgb_row))),(void*)((char*)src_rows+(i*sizeof(struct razer_rgb_row))),sizeof(struct razer_rgb_row));
				razer_copy_row(src_frame->rows[i],dst_frame->rows[i]);
			}
	}
}

void razer_set_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color)
{
	if(column_index<0 || column_index>=frame->columns_num)
		return;
	int y;
	for(y=0;y<frame->rows_num;y++)
	{
		frame->rows[y]->column[column_index].r = color->r;
		frame->rows[y]->column[column_index].g = color->g;
		frame->rows[y]->column[column_index].b = color->b;
		frame->update_mask |= 1<<y;
	}
	//frame->update_mask = 63;
}

void razer_add_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color)
{
	int r,g,b;
	if(column_index<0 || column_index>=frame->columns_num)
		return;

	int y;
	for(y=0;y<frame->rows_num;y++)
	{
		r = frame->rows[y]->column[column_index].r+color->r;
		g = frame->rows[y]->column[column_index].g+color->g;
		b = frame->rows[y]->column[column_index].b+color->b;
		if(r>255)
			r=255;
		if(g>255)
			g=255;
		if(b>255)
			b=255;
		frame->rows[y]->column[column_index].r = (unsigned char)r;
		frame->rows[y]->column[column_index].g = (unsigned char)g;
		frame->rows[y]->column[column_index].b = (unsigned char)b;
		frame->update_mask |= 1<<y;
	}
	//frame->update_mask = 63;
}

void razer_sub_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color)
{
	int r,g,b;
	if(column_index<0 || column_index>=frame->columns_num)
		return;
	int y;
	for(y=0;y<frame->rows_num;y++)
	{
		r = frame->rows[y]->column[column_index].r-color->r;
		g = frame->rows[y]->column[column_index].g-color->g;
		b = frame->rows[y]->column[column_index].b-color->b;
		if(r<0)
			r=0;
		if(g<0)
			g=0;
		if(b<0)
			b=0;
		frame->rows[y]->column[column_index].r = (unsigned char)r;
		frame->rows[y]->column[column_index].g = (unsigned char)g;
		frame->rows[y]->column[column_index].b = (unsigned char)b;
		frame->update_mask |= 1<<y;
	}
	//frame->update_mask = 63;
}

void razer_mix_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color,float opacity)
{
	if(column_index<0 || column_index>frame->columns_num)
		return;
	int y;
	for(y=0;y<frame->rows_num;y++)
	{
		frame->rows[y]->column[column_index].r = rgb_clamp((1.0f-opacity)*frame->rows[y]->column[column_index].r + color->r*opacity);
		frame->rows[y]->column[column_index].g = rgb_clamp((1.0f-opacity)*frame->rows[y]->column[column_index].g + color->g*opacity);
		frame->rows[y]->column[column_index].b = rgb_clamp((1.0f-opacity)*frame->rows[y]->column[column_index].b + color->b*opacity);
		frame->update_mask |= 1<<y;
	}
	//frame->update_mask = 63;
}

void razer_mix_frames(struct razer_rgb_frame *dst_frame,struct razer_rgb_frame *src_frame,float opacity)
{
	int x,y;
	int w,h;
	w = dst_frame->columns_num;
	h = dst_frame->rows_num;
	if(src_frame->columns_num<w)
		w = src_frame->columns_num;
	if(src_frame->rows_num<h)
		h = src_frame->rows_num;

	for(y=0;y<w;y++)
		for(x=0;x<h;x++)
		{
			//rgb_mix_into(&dst_frame->rows[y].column[x],&dst_frame->rows[y].column[x],&src_frame->rows[y].column[x],opacity);
			rgb_mix_into(&dst_frame->rows[y]->column[x],&src_frame->rows[y]->column[x],&dst_frame->rows[y]->column[x],opacity);
			dst_frame->update_mask |= 1<<y;
		}
	//dst_frame->update_mask = 63;
}

void razer_set_frame_row(struct razer_rgb_frame *frame,int row_index,struct razer_rgb *color)
{
	if(row_index<0 || row_index>=frame->rows_num)
		return;
	int x;
	for(x=0;x<frame->columns_num;x++)
	{
		frame->rows[row_index]->column[x].r = color->r;
		frame->rows[row_index]->column[x].g = color->g;
		frame->rows[row_index]->column[x].b = color->b;
	}
	frame->update_mask |= 1<<row_index;
}

void razer_add_frame_row(struct razer_rgb_frame *frame,int row_index,struct razer_rgb *color)
{
	int r,g,b;
	if(row_index<0 || row_index>=frame->rows_num)
		return;
	int x;
	int columns_num = frame->columns_num;
	for(x=0;x<columns_num;x++)
	{
		r = frame->rows[row_index]->column[x].r+color->r;
		g = frame->rows[row_index]->column[x].g+color->g;
		b = frame->rows[row_index]->column[x].b+color->b;
		if(r>255)
			r=255;
		if(g>255)
			g=255;
		if(b>255)
			b=255;
		frame->rows[row_index]->column[x].r = (unsigned char)r;
		frame->rows[row_index]->column[x].g = (unsigned char)g;
		frame->rows[row_index]->column[x].b = (unsigned char)b;
	}
	frame->update_mask |= 1<<row_index;
}

void razer_sub_frame_row(struct razer_rgb_frame *frame,int row_index,struct razer_rgb *color)
{
	int r,g,b;
	if(row_index<0 || row_index>=frame->rows_num)
		return;

	int x;
	int columns_num = frame->columns_num;
	for(x=0;x<columns_num;x++)
	{
		r = frame->rows[row_index]->column[x].r+color->r;
		g = frame->rows[row_index]->column[x].g+color->g;
		b = frame->rows[row_index]->column[x].b+color->b;
		if(r>255)
			r=255;
		if(g>255)
			g=255;
		if(b>255)
			b=255;
		frame->rows[row_index]->column[x].r = (unsigned char)r;
		frame->rows[row_index]->column[x].g = (unsigned char)g;
		frame->rows[row_index]->column[x].b = (unsigned char)b;
	}
	frame->update_mask |= 1<<row_index;
}

void razer_set_led(struct razer_rgb_frame *frame,int column_index,int row_index,struct razer_rgb *color)
{
	if(row_index<0 || row_index>=frame->rows_num)
		return;
	if(column_index<0 || column_index>=frame->columns_num)
		return;
	frame->rows[row_index]->column[column_index].r = color->r;
	frame->rows[row_index]->column[column_index].g = color->g;
	frame->rows[row_index]->column[column_index].b = color->b;
	frame->update_mask |= 1<<row_index;
}

void razer_set_led_pos(struct razer_rgb_frame *frame,struct razer_pos *pos,struct razer_rgb *color)
{
	int row_index = pos->y;
	int column_index = pos->x;
	if(row_index<0 || row_index>=frame->rows_num)
		return;
	if(column_index<0 || column_index>=frame->columns_num)
		return;
	frame->rows[row_index]->column[column_index].r = color->r;
	frame->rows[row_index]->column[column_index].g = color->g;
	frame->rows[row_index]->column[column_index].b = color->b;
	frame->update_mask |= 1<<row_index;
}

void razer_clear_all(struct razer_rgb_frame *frame)
{
	struct razer_rgb color = {.r=0,.g=0,.b=0};
	int i;
	for(i=0;i<frame->rows_num;i++)
		razer_set_frame_row(frame,i,&color);
}

void razer_set_all(struct razer_rgb_frame *frame,struct razer_rgb *color)
{
	int i;
	for(i=0;i<frame->rows_num;i++)
		razer_set_frame_row(frame,i,color);
}

/*void set_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color)
{
	if(column_index<0 || column_index>21)
		return;
	int y;
	for(y=0;y<6;y++)
	{
		keys->rows[y].column[column_index].r = color->r;
		keys->rows[y].column[column_index].g = color->g;
		keys->rows[y].column[column_index].b = color->b;
	}
	keys->update_mask = 63;
}

void add_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color)
{
	int r,g,b;
	if(column_index<0 || column_index>21)
		return;

	int y;
	for(y=0;y<6;y++)
	{
		r = keys->rows[y].column[column_index].r+color->r;
		g = keys->rows[y].column[column_index].g+color->g;
		b = keys->rows[y].column[column_index].b+color->b;
		if(r>255)
			r=255;
		if(g>255)
			g=255;
		if(b>255)
			b=255;
		keys->rows[y].column[column_index].r = (unsigned char)r;
		keys->rows[y].column[column_index].g = (unsigned char)g;
		keys->rows[y].column[column_index].b = (unsigned char)b;
	}
	keys->update_mask = 63;
}

void sub_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color)
{
	int r,g,b;
	if(column_index<0 || column_index>21)
		return;
	int y;
	for(y=0;y<6;y++)
	{
		r = keys->rows[y].column[column_index].r-color->r;
		g = keys->rows[y].column[column_index].g-color->g;
		b = keys->rows[y].column[column_index].b-color->b;
		if(r<0)
			r=0;
		if(g<0)
			g=0;
		if(b<0)
			b=0;
		keys->rows[y].column[column_index].r = (unsigned char)r;
		keys->rows[y].column[column_index].g = (unsigned char)g;
		keys->rows[y].column[column_index].b = (unsigned char)b;
	}
	keys->update_mask = 63;
}
*/

/*void set_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color)
{
	if(row_index<0 || row_index>5)
		return;
	int x;
	for(x=0;x<22;x++)
	{
		keys->rows[row_index].column[x].r = color->r;
		keys->rows[row_index].column[x].g = color->g;
		keys->rows[row_index].column[x].b = color->b;
	}
	keys->update_mask |= 1<<row_index;
}

void add_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color)
{
	int r,g,b;
	if(row_index<0 || row_index>5)
		return;
	int x;
	for(x=0;x<22;x++)
	{
		r = keys->rows[row_index].column[x].r+color->r;
		g = keys->rows[row_index].column[x].g+color->g;
		b = keys->rows[row_index].column[x].b+color->b;
		if(r>255)
			r=255;
		if(g>255)
			g=255;
		if(b>255)
			b=255;
		keys->rows[row_index].column[x].r = (unsigned char)r;
		keys->rows[row_index].column[x].g = (unsigned char)g;
		keys->rows[row_index].column[x].b = (unsigned char)b;
	}
	keys->update_mask |= 1<<row_index;
}

void sub_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color)
{
	int r,g,b;
	if(row_index<0 || row_index>5)
		return;

	int x;
	for(x=0;x<22;x++)
	{
		r = keys->rows[row_index].column[x].r+color->r;
		g = keys->rows[row_index].column[x].g+color->g;
		b = keys->rows[row_index].column[x].b+color->b;
		if(r>255)
			r=255;
		if(g>255)
			g=255;
		if(b>255)
			b=255;
		keys->rows[row_index].column[x].r = (unsigned char)r;
		keys->rows[row_index].column[x].g = (unsigned char)g;
		keys->rows[row_index].column[x].b = (unsigned char)b;
	}
	keys->update_mask |= 1<<row_index;
}

void razer_set_key(struct razer_keys *keys,int column_index,int row_index,struct razer_rgb *color)
{
	if(row_index<0 || row_index>6)
		return;
	if(column_index<0 || column_index>21)
		return;
	keys->rows[row_index].column[column_index].r = color->r;
	keys->rows[row_index].column[column_index].g = color->g;
	keys->rows[row_index].column[column_index].b = color->b;
	keys->update_mask |= 1<<row_index;
}

void razer_set_key_pos(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color)
{
	int row_index = pos->y;
	int column_index = pos->x;
	if(row_index<0 || row_index>6)
		return;
	if(column_index<0 || column_index>21)
		return;
	keys->rows[row_index].column[column_index].r = color->r;
	keys->rows[row_index].column[column_index].g = color->g;
	keys->rows[row_index].column[column_index].b = color->b;
	keys->update_mask |= 1<<row_index;
}

void razer_clear_all(struct razer_keys *keys)
{
	struct razer_rgb color = {.r=0,.g=0,.b=0};
	int i;
	for(i=0;i<6;i++)
		set_keys_row(keys,i,&color);
}

void razer_set_all(struct razer_keys *keys,struct razer_rgb *color)
{
	int i;
	for(i=0;i<6;i++)
		set_keys_row(keys,i,color);
}*/
/*
void sub_heatmap(struct razer_keys *keys,int heatmap_reduction_amount)
{
	int x,y;
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
			keys->heatmap[y][x]-=heatmap_reduction_amount;
		}
	}
}
*/
void razer_draw_line(struct razer_rgb_frame *frame,struct razer_pos *a,struct razer_pos *b,struct razer_rgb *color)
{
	int dx = abs(b->x-a->x);
	int dy = -abs(b->y-a->y);
	int sx = 1;
	int sy = 1;
	if(a->x>b->x)
		sx=-1;
	if(a->y>b->y)
		sy=-1;
	int e = dx+dy;
	int e2;
	struct razer_pos pos;
	pos.x = a->x;
	pos.y = a->y;

	while(1)
	{
		razer_set_led_pos(frame,&pos,color);
		if(pos.x == b->x && pos.y == b->y)
			break;
		e2 = 2*e;
		if(e2>dy)
		{
			e += dy;
			pos.x += sx;
		}
		if(e2<dx)
		{
			e += dx;
			pos.y += sy;
		}
	}
}

void razer_draw_circle(struct razer_rgb_frame *frame,struct razer_pos *pos,int radius,struct razer_rgb *color)
{
	int x = radius;
	int y= 0;
	int re = 1-x;
	while( x>= y)
	{
		razer_set_led(frame,x+pos->x, y + pos->y,color);
		razer_set_led(frame,y+pos->x, x + pos->y,color);
		razer_set_led(frame,-x+pos->x, y + pos->y,color);
		razer_set_led(frame,-y+pos->x, x + pos->y,color);
		razer_set_led(frame,-x+pos->x, -y + pos->y,color);
		razer_set_led(frame,-y+pos->x, -x + pos->y,color);
		razer_set_led(frame,x+pos->x, -y + pos->y,color);
		razer_set_led(frame,y+pos->x, -x + pos->y,color);
		y++;
		if(re < 0)
			re += 2*y+1;
		else
		{
			x--;
			re += 2*(y-x+1);
		}
	}
}

void razer_draw_ring(struct razer_rgb_frame *frame,struct razer_pos *pos,struct razer_rgb *color)
{
	razer_set_led(frame,pos->x+1, pos->y,color);
	razer_set_led(frame,pos->x-1, pos->y,color);
	if(pos->y==4 || pos->y==1)
	{
		razer_set_led(frame,pos->x-1, pos->y-1,color);
		razer_set_led(frame,pos->x, pos->y-1,color);
		razer_set_led(frame,pos->x-1, pos->y+1,color);
		razer_set_led(frame,pos->x, pos->y+1,color);
	}
	else
	{
		razer_set_led(frame,pos->x, pos->y-1,color);
		razer_set_led(frame,pos->x+1, pos->y-1,color);
		razer_set_led(frame,pos->x, pos->y+1,color);
		razer_set_led(frame,pos->x+1, pos->y+1,color);
	}
}

//TODO
//list of last keystrokes
//time since hit /hitstamps

double deg2rad(double degree)
{
  double rad=degree*(PI/180);
  return(rad);
}

double rad2deg(double rad)
{
  double deg=rad*(180/PI);
  return(deg);
}


double pos_angle_radians(struct razer_pos *src,struct razer_pos *dst)
{
	double x,y;
	x=(double)src->x - (double)dst->x;
	y=(double)src->y - (double)dst->y;
	double angle = atan2(x,y);
	return(angle);
}


void razer_set_event_handler(struct razer_chroma *chroma,razer_event_handler handler)
{
	//set the input handler callback which will receive events from input devices and dbus functions (TODO)
	chroma->event_handler = handler;
}

void razer_copy_pos(struct razer_pos *src, struct razer_pos *dst)
{
	dst->x = src->x;
	dst->y = src->y;
}

unsigned long razer_get_ticks()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long ms = tv.tv_usec/1000 + tv.tv_sec * 1000;
	return(ms);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

void razer_frame_limiter(struct razer_chroma *chroma,int fps)
{
	long diff_ms = chroma->update_ms - chroma->last_update_ms;
	int wanted_ms = 1000/fps;
	if(diff_ms<wanted_ms)
	{
		usleep((wanted_ms-diff_ms)*1000);
	}
	//chroma->last_update_ms = chroma->update_ms;
	chroma->last_update_ms = razer_get_ticks(); //TODO too hidden .. move or explain
}

#pragma GCC diagnostic pop

void razer_update(struct razer_chroma *chroma)
{
	chroma->update_ms = razer_get_ticks();
	long diff_ms = chroma->update_ms - chroma->last_update_ms;
	chroma->update_dt = (float)diff_ms / 1000.0f;
	if(chroma->event_handler)
		razer_poll_input_devices(chroma);
}




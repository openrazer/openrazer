#include "razer_chroma.h"

char *razer_sys_hid_devices_path = "/sys/bus/hid/devices/";
char *razer_sys_keyboard_event_path = "/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd";
//char *razer_sys_mouse_event_path = "/dev/input/mouse0";
//char *razer_sys_mouse_event_path = "/dev/input/mouse2";
char *razer_sys_mouse_event_path = "/dev/input/by-id/usb-ROCCAT_ROCCAT_Kone_Pure-event-mouse";

char *razer_custom_mode_pathname = "/mode_custom";
char *razer_update_keys_pathname = "/set_key_row";

/*some string routines from node.c*/
char *str_CreateEmpty(void)
{
    char *string = (char*)malloc(1);
    string[0] = 0;
    return(string);
}

char *str_Copy(char *src)
{
  if(src==NULL)
  	return(str_CreateEmpty());
  char *a = (char*)malloc(strlen(src)+1);
  memcpy(a, src, strlen(src)+1);
  return(a);
}

char *str_Cat(char *a,char *b)
{
  if(a == NULL && b != NULL)
    return(str_Copy(b));
  else
    if(a != NULL && b == NULL)
      return(str_Copy(a));
  else
    if(a == NULL && b == NULL)
      return(str_CreateEmpty());
  char *tmp = (char*)malloc(strlen(a) + strlen(b) + 1);
  memcpy(tmp, a, strlen(a));
  memcpy(tmp + strlen(a), b, strlen(b)+1);
  return(tmp);
}

char *str_CatFree(char *a,char *b)
{
  if(a == NULL && b != NULL)
    return(str_Copy(b));
  else
    if(a != NULL && b == NULL)
      return(a);
  else
    if(a == NULL && b == NULL)
      return(str_CreateEmpty());
  char *tmp = (char*)malloc(strlen(a) + strlen(b) + 1);
  memcpy(tmp, a, strlen(a));
  memcpy(tmp + strlen(a), b, strlen(b)+1);
  free(a);
  return(tmp);
}

char *str_FromLong(long i)
{
  char *ret=NULL;
  long len = snprintf(NULL,0,"%ld",i);
  if(len)
  {
    ret = (char*)malloc(len+1);
    snprintf(ret,len+1,"%ld",i);
  }
  else
    ret=str_CreateEmpty();
  return(ret);
} 

char *str_FromDouble(double d)
{
  char *ret=NULL;
  long len = snprintf(NULL,0,"%f",d);
  if(len)
  {
    ret = (char*)malloc(len+1);
    snprintf(ret,len+1,"%f",d);
  }
  else
    ret=str_CreateEmpty();
  return(ret);
} 


int razer_open_custom_mode_file(struct razer_chroma *chroma)
{
	chroma->custom_mode_file=fopen(chroma->custom_mode_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening custom mode file:%s\n",chroma->custom_mode_filename);
	#endif
	if(chroma->custom_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_custom_mode_file(struct razer_chroma *chroma)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing custom mode file:%s\n",chroma->custom_mode_filename);
	#endif
    if(chroma->custom_mode_file)
    	fclose(chroma->custom_mode_file);
    chroma->custom_mode_file = NULL;
}

int razer_open_update_keys_file(struct razer_chroma *chroma)
{
	chroma->update_keys_file=fopen(chroma->update_keys_filename,"wb");
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening update keys file:%s\n",chroma->update_keys_filename);
	#endif
	if(chroma->update_keys_file)
		return(1);
	else
		return(0);
}

void razer_close_update_keys_file(struct razer_chroma *chroma)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing update keys file:%s\n",chroma->update_keys_filename);
	#endif
    if(chroma->update_keys_file)
    	fclose(chroma->update_keys_file);
    chroma->update_keys_file = NULL;
}

int razer_open_keyboard_input_file(struct razer_chroma *chroma)
{
	chroma->keyboard_input_file=open(razer_sys_keyboard_event_path,O_RDONLY | O_NONBLOCK | O_NOCTTY | O_NDELAY);
	fcntl(chroma->keyboard_input_file,F_SETFL,0);
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening keyboard input file:%s\n",razer_sys_keyboard_event_path);
	#endif
	if(chroma->keyboard_input_file)
		return(1);
	else
		return(0);
}

void razer_close_keyboard_input_file(struct razer_chroma *chroma)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing keyboard input file:%s\n",razer_sys_keyboard_event_path);
	#endif
    if(chroma->keyboard_input_file)
    	close(chroma->keyboard_input_file);
    chroma->keyboard_input_file = 0;
}

int razer_open_mouse_input_file(struct razer_chroma *chroma)
{
	chroma->mouse_input_file=open(razer_sys_mouse_event_path,O_RDONLY | O_NONBLOCK | O_NOCTTY | O_NDELAY);
	fcntl(chroma->mouse_input_file,F_SETFL,0);
	#ifdef USE_VERBOSE_DEBUGGING
		printf("opening mouse input file:%s\n",razer_sys_mouse_event_path);
	#endif
	if(chroma->mouse_input_file)
		return(1);
	else
		return(0);
}

void razer_close_mouse_input_file(struct razer_chroma *chroma)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("closing mouse input file:%s\n",razer_sys_mouse_event_path);
	#endif
    if(chroma->mouse_input_file)
    	close(chroma->mouse_input_file);
    chroma->mouse_input_file = 0;
}

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

void razer_init_frame(struct razer_rgb_frame *frame)
{
	memset(frame->rows,0,sizeof(struct razer_rgb_row)*6);
	frame->update_mask = 63;
	int i;
	for(i = 0; i < 6; ++i)
	{
		frame->rows[i].row_index = i;
	}
}

struct razer_chroma *razer_open(void)
{
	struct razer_chroma *chroma =(struct razer_chroma*)malloc(sizeof(struct razer_chroma));
	#ifdef USE_DEBUGGING
		printf("opening chroma lib\n");
	#endif
	chroma->custom_mode_file = NULL;
	chroma->update_keys_file = NULL;
	chroma->keyboard_input_file = 0;
	chroma->mouse_input_file = 0;
	chroma->device_path = razer_get_device_path();
	if(!chroma->device_path)
	{
		#ifdef USE_DEBUGGING
			printf("error no compatible device found\n");
		#endif
		return(NULL);
	}
	#ifdef USE_VERBOSE_DEBUGGING
		printf("found device at path:%s\n",chroma->device_path);
	#endif

	chroma->keys = (struct razer_keys*)malloc(sizeof(struct razer_keys));
	razer_init_keys(chroma->keys);
	chroma->custom_mode_filename = str_CreateEmpty();
	chroma->custom_mode_filename = str_CatFree(chroma->custom_mode_filename,chroma->device_path);
	chroma->custom_mode_filename = str_CatFree(chroma->custom_mode_filename,razer_custom_mode_pathname);

	chroma->update_keys_filename = str_CreateEmpty();
	chroma->update_keys_filename = str_CatFree(chroma->update_keys_filename,chroma->device_path);
	chroma->update_keys_filename = str_CatFree(chroma->update_keys_filename,razer_update_keys_pathname);
	
	chroma->input_handler = NULL;
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
	if(chroma->device_path)
	{
		chroma->input_handler = NULL;
		free(chroma->keys);
		free(chroma->device_path);
		if(chroma->update_keys_file)
			razer_close_update_keys_file(chroma);
		if(chroma->custom_mode_file)
			razer_close_custom_mode_file(chroma);
		if(chroma->keyboard_input_file)
			razer_close_keyboard_input_file(chroma);
		if(chroma->mouse_input_file)
			razer_close_mouse_input_file(chroma);
		free(chroma->custom_mode_filename);
		free(chroma->update_keys_filename);
	}
	free(chroma);
}

char *razer_get_device_path()
{
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
		if(device_vendor_id==RAZER_VENDOR_ID && device_product_id == RAZER_BLACKWIDOW_CHROMA_PRODUCT_ID)
		{
			char *custom_path_name = "/mode_custom";
			int base_path_len = strlen(razer_sys_hid_devices_path)+strlen(entry->d_name);
			int custom_path_len = base_path_len+strlen(custom_path_name);
			char *custom_filename = (char*)malloc(custom_path_len+1);
			char *device_path = (char*)malloc(base_path_len+1);
			memset(custom_filename,0,custom_path_len+1);			
			memcpy(custom_filename,razer_sys_hid_devices_path,strlen(razer_sys_hid_devices_path));
			memcpy(custom_filename+strlen(razer_sys_hid_devices_path),entry->d_name,strlen(entry->d_name));
			memcpy(custom_filename+strlen(razer_sys_hid_devices_path)+strlen(entry->d_name),custom_path_name,strlen(custom_path_name));

			memset(device_path,0,custom_path_len+1);			
			memcpy(device_path,razer_sys_hid_devices_path,strlen(razer_sys_hid_devices_path));
			memcpy(device_path+strlen(razer_sys_hid_devices_path),entry->d_name,strlen(entry->d_name));
			FILE *fmode=fopen(custom_filename,"wt");
			if(fmode)
			{
				fclose(fmode);
				closedir(d);
				free(custom_filename);
				return(device_path);
			}
		}
	}
	closedir(d);
	return(NULL);
}


struct razer_rgb_frame *razer_create_rgb_frame(void)
{
	struct razer_rgb_frame *frame = (struct razer_rgb_frame*)malloc(sizeof(struct razer_rgb_frame));
	razer_init_frame(frame);
	return(frame);
}

void razer_free_rgb_frame(struct razer_rgb_frame *frame)
{
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


void release_locks(struct razer_keys_locks *locks)
{
	memset(locks,0,sizeof(struct razer_keys_locks));
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

void razer_set_custom_mode(struct razer_chroma *chroma)
{
	if(!chroma->custom_mode_file)
		razer_open_custom_mode_file(chroma);
	if(chroma->custom_mode_file)
		fwrite("1",1,1,chroma->custom_mode_file);
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to custom\n");
	#endif
	fflush(chroma->custom_mode_file);
}

void razer_update_keys(struct razer_chroma *chroma, struct razer_keys *keys)
{
	int i;
	if(!keys->update_mask)
		return;
	if(!chroma->update_keys_file)
		razer_open_update_keys_file(chroma);
	if(chroma->update_keys_file)
	{
		for(i=0;i<6;i++)
		{
			if(keys->update_mask &(1<<i))
				fwrite((void*)&keys->rows[i],sizeof(struct razer_rgb_row),1,chroma->update_keys_file);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to custom\n");
	#endif
	fflush(chroma->update_keys_file);
   	razer_set_custom_mode(chroma);
    keys->update_mask=0;
}

void razer_update_frame(struct razer_chroma *chroma, struct razer_rgb_frame *frame)
{
	int i;
	if(!frame->update_mask)
		return;
	if(!chroma->update_keys_file)
		razer_open_update_keys_file(chroma);
	if(chroma->update_keys_file)
	{
		for(i=0;i<6;i++)
		{
			if(frame->update_mask &(1<<i))
				fwrite((void*)&frame->rows[i],sizeof(struct razer_rgb_row),1,chroma->update_keys_file);
		}
	}
	#ifdef USE_DEBUGGING
	else
		printf("error setting mode to custom\n");
	#endif
	fflush(chroma->update_keys_file);
   	razer_set_custom_mode(chroma);
    frame->update_mask=0;
}

void razer_clear_frame(struct razer_rgb_frame *frame)
{
	int i;
	for(i=0;i<6;i++)
		memset((void*)((char*)frame->rows+(i*sizeof(struct razer_rgb_row))),0,sizeof(struct razer_rgb_row));
	frame->update_mask = 0;
}

void razer_set_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color)
{
	if(column_index<0 || column_index>21)
		return;
	int y;
	for(y=0;y<6;y++)
	{
		frame->rows[y].column[column_index].r = color->r;
		frame->rows[y].column[column_index].g = color->g;
		frame->rows[y].column[column_index].b = color->b;
	}
	frame->update_mask = 63;
}

void razer_add_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color)
{
	int r,g,b;
	if(column_index<0 || column_index>21)
		return;

	int y;
	for(y=0;y<6;y++)
	{
		r = frame->rows[y].column[column_index].r+color->r;
		g = frame->rows[y].column[column_index].g+color->g;
		b = frame->rows[y].column[column_index].b+color->b;
		if(r>255)
			r=255;
		if(g>255)
			g=255;
		if(b>255)
			b=255;
		frame->rows[y].column[column_index].r = (unsigned char)r;
		frame->rows[y].column[column_index].g = (unsigned char)g;
		frame->rows[y].column[column_index].b = (unsigned char)b;
	}
	frame->update_mask = 63;
}

void razer_sub_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color)
{
	int r,g,b;
	if(column_index<0 || column_index>21)
		return;
	int y;
	for(y=0;y<6;y++)
	{
		r = frame->rows[y].column[column_index].r-color->r;
		g = frame->rows[y].column[column_index].g-color->g;
		b = frame->rows[y].column[column_index].b-color->b;
		if(r<0)
			r=0;
		if(g<0)
			g=0;
		if(b<0)
			b=0;
		frame->rows[y].column[column_index].r = (unsigned char)r;
		frame->rows[y].column[column_index].g = (unsigned char)g;
		frame->rows[y].column[column_index].b = (unsigned char)b;
	}
	frame->update_mask = 63;
}

void razer_mix_frame_column(struct razer_rgb_frame *frame,int column_index,struct razer_rgb *color,float opacity)
{
	if(column_index<0 || column_index>21)
		return;
	int y;
	for(y=0;y<6;y++)
	{
		frame->rows[y].column[column_index].r = rgb_clamp((1.0f-opacity)*frame->rows[y].column[column_index].r + color->r*opacity);
		frame->rows[y].column[column_index].g = rgb_clamp((1.0f-opacity)*frame->rows[y].column[column_index].g + color->g*opacity);
		frame->rows[y].column[column_index].b = rgb_clamp((1.0f-opacity)*frame->rows[y].column[column_index].b + color->b*opacity);
	}
	frame->update_mask = 63;
}

void razer_mix_frames(struct razer_rgb_frame *dst_frame,struct razer_rgb_frame *src_frame,float opacity)
{
	int x,y;
	for(y=0;y<6;y++)
		for(x=0;x<22;x++)
		{
			//rgb_mix_into(&dst_frame->rows[y].column[x],&dst_frame->rows[y].column[x],&src_frame->rows[y].column[x],opacity);
			rgb_mix_into(&dst_frame->rows[y].column[x],&src_frame->rows[y].column[x],&dst_frame->rows[y].column[x],opacity);
		}
	dst_frame->update_mask = 63;
}

void razer_copy_rows(struct razer_rgb_row *src_rows,struct razer_rgb_row *dst_rows,int update_mask,int use_update_mask)
{
	int i;
	if(use_update_mask)
	{
		for(i=0;i<6;i++)
			if(update_mask &(1<<i))
			{
				memcpy((void*)((char*)dst_rows+(i*sizeof(struct razer_rgb_row))),(void*)((char*)src_rows+(i*sizeof(struct razer_rgb_row))),sizeof(struct razer_rgb_row));
			}
	}
	else
	{
		memcpy((void*)dst_rows,(void*)src_rows,sizeof(struct razer_rgb_row)*6);
	}
}

void set_keys_column(struct razer_keys *keys,int column_index,struct razer_rgb *color)
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

void set_keys_row(struct razer_keys *keys,int row_index,struct razer_rgb *color)
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
}

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

void draw_line(struct razer_keys *keys,struct razer_pos *a,struct razer_pos *b,struct razer_rgb *color)
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
		razer_set_key_pos(keys,&pos,color);
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

void draw_circle(struct razer_keys *keys,struct razer_pos *pos,int radius,struct razer_rgb *color)
{
	int x = radius;
	int y= 0;
	int re = 1-x;
	while( x>= y)
	{
		razer_set_key(keys,x+pos->x, y + pos->y,color);
		razer_set_key(keys,y+pos->x, x + pos->y,color);
		razer_set_key(keys,-x+pos->x, y + pos->y,color);
		razer_set_key(keys,-y+pos->x, x + pos->y,color);
		razer_set_key(keys,-x+pos->x, -y + pos->y,color);
		razer_set_key(keys,-y+pos->x, -x + pos->y,color);
		razer_set_key(keys,x+pos->x, -y + pos->y,color);
		razer_set_key(keys,y+pos->x, -x + pos->y,color);
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

void draw_ring(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color)
{
	razer_set_key(keys,pos->x+1, pos->y,color);
	razer_set_key(keys,pos->x-1, pos->y,color);
	if(pos->y==4 || pos->y==1)
	{
		razer_set_key(keys,pos->x-1, pos->y-1,color);
		razer_set_key(keys,pos->x, pos->y-1,color);
		razer_set_key(keys,pos->x-1, pos->y+1,color);
		razer_set_key(keys,pos->x, pos->y+1,color);
	}
	else
	{
		razer_set_key(keys,pos->x, pos->y-1,color);
		razer_set_key(keys,pos->x+1, pos->y-1,color);
		razer_set_key(keys,pos->x, pos->y+1,color);
		razer_set_key(keys,pos->x+1, pos->y+1,color);
	}
}

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


void razer_set_input_handler(struct razer_chroma *chroma,razer_input_handler handler)
{
	chroma->input_handler = handler;
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
	struct timeval select_tv;
	struct razer_chroma_event chroma_event;
	int keycode = -1;
	if(!chroma->keyboard_input_file)
		razer_open_keyboard_input_file(chroma);
	if(!chroma->mouse_input_file)
		razer_open_mouse_input_file(chroma);
	fd_set keyboard_rs;
	fd_set mouse_rs;
	select_tv.tv_sec = 0;
	select_tv.tv_usec = 0;
	int r;
	char buf[2048];
	chroma->update_ms = razer_get_ticks();
	long diff_ms = chroma->update_ms - chroma->last_update_ms;
	chroma->update_dt = (float)diff_ms / 1000.0f;
	FD_ZERO(&keyboard_rs);
	FD_SET(chroma->keyboard_input_file,&keyboard_rs);
	r = select(chroma->keyboard_input_file+1,&keyboard_rs,0,0,&select_tv);
	if(r && FD_ISSET(chroma->keyboard_input_file,&keyboard_rs))
	{
		int n=2048;
		n = read(chroma->keyboard_input_file,buf,2048);
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
			razer_close_keyboard_input_file(chroma);
			return;
		}
		else				
		{
			unsigned int i;
			for(i=0;i<n/sizeof(struct input_event);i++)
			{
				struct input_event *event = (struct input_event*)(buf+(i*sizeof(struct input_event)));
				if(event->type==EV_KEY)
				{
					keycode = event->code;
					if(event->value==1)
					{
						/*if(keycode!=last_keycode)
						if(keycode==183)
							actual_mode=0;
						if(keycode==184)
							actual_mode=1;
						if(keycode==185)
							actual_mode=2;
						if(keycode==186)
							actual_mode=3;
						if(keycode==187)
							actual_mode=4;
						*/
						razer_copy_pos(&chroma->key_pos,&chroma->last_key_pos);
						razer_convert_keycode_to_pos(event->code,&chroma->key_pos);							
						chroma->keys->heatmap[chroma->key_pos.y][chroma->key_pos.x]+=1;
						chroma_event.key = "Key Down";
					}
					else
						chroma_event.key = "Key Up";
					if(chroma->input_handler)
					{
						chroma_event.type = RAZER_CHROMA_EVENT_TYPE_KEYBOARD;
						chroma_event.sub_type = event->value;
						chroma_event.value = keycode;

						chroma->input_handler(chroma,&chroma_event);
					}
					long key_diff_ms = chroma->update_ms - chroma->last_key_event_ms;
					chroma->key_event_dt = (float)key_diff_ms / 1000.0f;
					chroma->last_key_event_ms = chroma->update_ms;			
				}
			}
		}
	}	

	FD_ZERO(&mouse_rs);
	FD_SET(chroma->mouse_input_file,&mouse_rs);
	select_tv.tv_sec = 0;
	select_tv.tv_usec = 0;
	r = select(chroma->mouse_input_file+1,&mouse_rs,0,0,&select_tv);
	if(r && FD_ISSET(chroma->mouse_input_file,&mouse_rs))
	{
		int n=2048;
		n = read(chroma->mouse_input_file,buf,2048);
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
			razer_close_mouse_input_file(chroma);
			return;
		}
		else				
		{
			unsigned int i;
			unsigned int event_size = sizeof(struct input_event);
			for(i=0;i<n/event_size;i++)
			{
				struct input_event *event = (struct input_event*)(buf+(i*sizeof(struct input_event)));
					if(chroma->input_handler)
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
						chroma->input_handler(chroma,&chroma_event);
					}
					long key_diff_ms = chroma->update_ms - chroma->last_key_event_ms;
					chroma->key_event_dt = (float)key_diff_ms / 1000.0f;
					chroma->last_key_event_ms = chroma->update_ms;			
			}
		}
	}	

}




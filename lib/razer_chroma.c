#include "razer_chroma.h"

const char *razer_sys_hid_devices_path = "/sys/bus/hid/devices/";

const char *razer_sys_event_path = "/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd";

const char *razer_custom_mode_pathname = "/mode_custom";
const char *razer_update_keys_pathname = "/set_key_row";

int razer_open_custom_mode_file(struct razer_chroma *chroma)
{
	chroma->custom_mode_file=fopen(chroma->custom_mode_filename,"wt");
	#ifdef USE_DEBUGGING
		printf("opening custom mode file:%s\n",chroma->custom_mode_filename);
	#endif
	if(chroma->custom_mode_file)
		return(1);
	else
		return(0);
}

void razer_close_custom_mode_file(struct razer_chroma *chroma)
{
	#ifdef USE_DEBUGGING
		printf("closing custom mode file:%s\n",chroma->custom_mode_filename);
	#endif
    if(chroma->custom_mode_file)
    	fclose(chroma->custom_mode_file);
    chroma->custom_mode_file = NULL;
}

int razer_open_update_keys_file(struct razer_chroma *chroma)
{
	chroma->update_keys_file=fopen(chroma->update_keys_filename,"wb");
	#ifdef USE_DEBUGGING
		printf("opening update keys file:%s\n",chroma->update_keys_filename);
	#endif
	if(chroma->update_keys_file)
		return(1);
	else
		return(0);
}

void razer_close_update_keys_file(struct razer_chroma *chroma)
{
	#ifdef USE_DEBUGGING
		printf("closing update keys file:%s\n",chroma->update_keys_filename);
	#endif
    if(chroma->update_keys_file)
    	fclose(chroma->update_keys_file);
    chroma->update_keys_file = NULL;
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

int razer_open(struct razer_chroma *chroma)
{
	#ifdef USE_DEBUGGING
		printf("opening chroma lib\n");
	#endif
	chroma->custom_mode_file = NULL;
	chroma->update_keys_file = NULL;
	chroma->device_path = razer_get_device_path();
	if(!chroma->device_path)
	{
		#ifdef USE_DEBUGGING
			printf("error no compatible device found\n");
		#endif
		return(0);
	}
	#ifdef USE_DEBUGGING
		printf("found device at path:%s\n",chroma->device_path);
	#endif

	chroma->keys = (struct razer_keys*)malloc(sizeof(struct razer_keys));
	razer_init_keys(chroma->keys);

	chroma->custom_mode_filename = (char*)malloc(strlen(chroma->device_path)+strlen(razer_custom_mode_pathname)+1);
	memset(chroma->custom_mode_filename,0,strlen(chroma->device_path)+strlen(razer_custom_mode_pathname)+1);
	memcpy(chroma->custom_mode_filename,chroma->device_path,strlen(chroma->device_path));
	memcpy(chroma->custom_mode_filename+strlen(chroma->device_path),razer_custom_mode_pathname,strlen(razer_custom_mode_pathname));
	chroma->update_keys_filename = (char*)malloc(strlen(chroma->device_path)+strlen(razer_update_keys_pathname)+1);
	memset(chroma->update_keys_filename,0,strlen(chroma->device_path)+strlen(razer_update_keys_pathname)+1);
	memcpy(chroma->update_keys_filename,chroma->device_path,strlen(chroma->device_path));
	memcpy(chroma->update_keys_filename+strlen(chroma->device_path),razer_update_keys_pathname,strlen(razer_update_keys_pathname));

	return(1);
}

void razer_close(struct razer_chroma *chroma)
{
	#ifdef USE_DEBUGGING
		printf("closing chroma lib\n");
	#endif
	if(chroma->device_path)
	{
		free(chroma->keys);
		free(chroma->device_path);
		if(chroma->update_keys_file)
			razer_close_update_keys_file(chroma);
		if(chroma->custom_mode_file)
			razer_close_custom_mode_file(chroma);
		free(chroma->custom_mode_filename);
		free(chroma->update_keys_filename);
	}
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

void convert_keycode_to_pos(int keycode,struct razer_pos *pos)
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
			printf("unknown key:%d\n",keycode);
	}
}

void convert_pos_to_keycode(struct razer_pos *pos,int *keycode)
{

}

void convert_ascii_to_pos(unsigned char letter,struct razer_pos *pos)
{


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
	razer_close_custom_mode_file(chroma);
}

void razer_update_keys(struct razer_chroma *chroma, struct razer_keys *keys)
{
	int i;
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
	if(keys->update_mask)
    	razer_set_custom_mode(chroma);
    keys->update_mask=0;
    razer_close_update_keys_file(chroma);
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

void set_key(struct razer_keys *keys,int column_index,int row_index,struct razer_rgb *color)
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

void set_key_pos(struct razer_keys *keys,struct razer_pos *pos,struct razer_rgb *color)
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


void clear_all(struct razer_keys *keys)
{
	struct razer_rgb color = {.r=0,.g=0,.b=0};
	int i;
	for(i=0;i<6;i++)
		set_keys_row(keys,i,&color);
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

void draw_circle(struct razer_keys *keys,struct razer_pos *pos,int radius,struct razer_rgb *color)
{
	int x = radius;
	int y= 0;
	int re = 1-x;
	while( x>= y)
	{
		set_key(keys,x+pos->x, y + pos->y,color);
		set_key(keys,y+pos->x, x + pos->y,color);
		set_key(keys,-x+pos->x, y + pos->y,color);
		set_key(keys,-y+pos->x, x + pos->y,color);
		set_key(keys,-x+pos->x, -y + pos->y,color);
		set_key(keys,-y+pos->x, -x + pos->y,color);
		set_key(keys,x+pos->x, -y + pos->y,color);
		set_key(keys,y+pos->x, -x + pos->y,color);
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

void draw_ring(struct razer_keys *keys,struct razer_pos *pos,int radius,struct razer_rgb *color)
{
	set_key(keys,pos->x+1, pos->y,color);
	set_key(keys,pos->x-1, pos->y,color);
	if(pos->y==4 || pos->y==1)
	{
		set_key(keys,pos->x-1, pos->y-1,color);
		set_key(keys,pos->x, pos->y-1,color);
		set_key(keys,pos->x-1, pos->y+1,color);
		set_key(keys,pos->x, pos->y+1,color);
	}
	else
	{
		set_key(keys,pos->x, pos->y-1,color);
		set_key(keys,pos->x+1, pos->y-1,color);
		set_key(keys,pos->x, pos->y+1,color);
		set_key(keys,pos->x+1, pos->y+1,color);
	}
}

SDL_Texture *sdl_icons[32];

void update_sdl(struct razer_keys *keys,SDL_Renderer *sdl,SDL_Window *window,SDL_Texture *tex)
{
	int x,y;
	unsigned char *pixels = (unsigned char*)malloc(4*22*6);
	for(x=0;x<22;x++)
	{
		for(y=0;y<6;y++)
		{
			pixels[((x+(y*22))*4)+3] = 255;
			pixels[((x+(y*22))*4)+0] = keys->rows[y].column[x].b;
			pixels[((x+(y*22))*4)+1] = keys->rows[y].column[x].g;
			pixels[((x+(y*22))*4)+2] = keys->rows[y].column[x].r;
		}
	}
	SDL_Rect rect;

	SDL_UpdateTexture(tex, NULL, pixels, 22*4);
	SDL_RenderClear(sdl);
	rect.x=0;
	rect.y=0;
	int w,h;
	SDL_GetWindowSize(window,&w,&h);
	rect.w=w;
	rect.h=h-32;
	SDL_RenderCopy(sdl,tex,NULL,&rect);

	int i;
	for(i=0;i<4;i++)
	{
		rect.x=10+i*26;
		rect.y=h-24;
		rect.w=16;
		rect.h=16;
		SDL_RenderCopy(sdl,sdl_icons[i],NULL,&rect);
	}


	SDL_RenderPresent(sdl);
}

void load_icons(SDL_Renderer *renderer,char *path,SDL_Texture **icons)
{

	//sdl_icons[0] = SDL_CreateTexture(sdl_renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,22,6);
    sdl_icons[0] = IMG_LoadTexture(renderer,"icons/IPencil.png");
    sdl_icons[1] = IMG_LoadTexture(renderer,"icons/ILink.png");
    sdl_icons[2] = IMG_LoadTexture(renderer,"icons/IWrench.png");
    sdl_icons[3] = IMG_LoadTexture(renderer,"icons/ITrash.png");

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

float scroll_x,scroll_y;
int scroll_width,scroll_height;
double scroll_dir_x,scroll_dir_y;
unsigned char *scroll_buf=NULL;

/*
void capture_keys(struct razer_keys *keys,SDL_Renderer *renderer,SDL_Window *window,SDL_Texture *tex)
{
	struct timeval start,tv,select_tv;
	gettimeofday(&start, NULL);
	int ev_count = 0;
	int finput=open(sys_event_path,O_RDONLY | O_NONBLOCK | O_NOCTTY | O_NDELAY);
	fcntl(finput,F_SETFL,0);
	int last_keycode=0;
	fd_set rs;
	select_tv.tv_sec = 0;
	select_tv.tv_usec = 0;
	int r;
	int actual_mode = 0;
	int last_event_count=0;
	int done = 0;
	long actual_ms = start.tv_usec;
	long last_ms = start.tv_usec;
	long diff_ms = 0;
	while(!done)
	{

		gettimeofday(&tv, NULL);
		last_ms = actual_ms;
		actual_ms = tv.tv_usec;
		diff_ms = actual_ms-last_ms;
		SDL_Event event;
	    while(SDL_PollEvent(&event)) 
    	{
		    if(event.type == SDL_KEYUP)
    		{
		    	if(event.key.keysym.sym == SDLK_ESCAPE)
			    	done=1;
	      	}
		    if(event.type == SDL_MOUSEBUTTONUP)
    		{
		    	//if(event.key.keysym.sym == SDLK_ESCAPE)
				int w,h;
				SDL_GetWindowSize(window,&w,&h);
				int kw=w/22;
				int kh=(h-32)/6;
		    	if(event.button.y<h-32)
		    	{
		    		int kx = (event.button.x)/kw;
		    		int ky = event.button.y/kh;
		    		//printf("button pressed in:%d,%d\n",kx,ky);
					struct razer_rgb cr = {.r=128,.g=0,.b=0};
					set_key(keys,kx,ky,&cr);
		    	}
			    //	done=1;

	      	}
		    if(event.type == SDL_QUIT)
    		{
		    	done=1;
	      	}

 		}

		struct razer_pos pos;
		FD_ZERO(&rs);
		FD_SET(finput,&rs);
		r = select(finput+1,&rs,0,0,&select_tv);
		//r = select(finput+1,&rs,0,0,0);
		//clear_all(keys);
		if(actual_mode==0)
			test_effect_frame(keys);
		else
			if(actual_mode==1)
				test_effect2_frame(keys);
			else
				if(actual_mode==3)
					//test_effect3_frame(keys);
					test_effect_heatmap_frame(keys);
				else
					if(actual_mode==4)
					{
		//				test_effect_heatmap_frame(keys);
						//clear_all(keys);
						test_effect_scroll_frame(keys);
					}

		//clear_all(keys);
		test_effect_scroll_frame(keys);
		//test_effect_heatmap_frame(keys);
		struct razer_rgb lcr = {.r=128,.g=0,.b=0};
		if(actual_mode!=3 && actual_mode != 4)
		{
			set_keys_row(keys,pos.y,&lcr);
			set_keys_column(keys,pos.x,&lcr);
		}
		if(actual_mode==1 || actual_mode == 2)
		{
			int x,y;
			for(x=0;x<22;x++)
			{
				for(y=0;y<6;y++)
				{
					struct razer_pos hpos = {.x=x,.y=y};
					//struct razer_rgb hcol = {.r=keys->heatmap[y][x],.g=random()%4*3,.b=0};
					struct razer_rgb hcol = {.r=keys->heatmap[y][x],.g=0,.b=0};
					if(keys->heatmap[y][x]||actual_mode==2)
						set_key_pos(keys,&hpos,&hcol);
					//if(keys->heatmap[y][x]>0)
					//	keys->heatmap[y][x]-=1;
				}
			}
		}

		

		if(!r)
		{
			update_keys(keys);
			update_sdl(keys,renderer,window,tex);
			continue;
		}
		char buf[2048];
		if(FD_ISSET(finput,&rs))
		{

			int n=2048;
			n = read(finput,buf,2048);
			if(n<0)
			{
				//if(errno == EAGAIN)
				//	printf("waiting for data\n");
			}
			else if(n==0)
			{
				close(finput);
				return;
			}
			else				
			{
				int i;
				for(i=0;i<n/sizeof(struct input_event);i++)
				{
					struct input_event *event = buf+(i*sizeof(struct input_event));
					if(event->type==EV_KEY)
					{
						if(event->value==1)
						{
							int keycode = event->code;
							//if(keycode!=last_keycode)
							{
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
								//printf("ev_code:%d\n",event->code);
								struct razer_rgb cr = {.r=128,.g=0,.b=0};
								struct razer_rgb cg = {.r=0,.g=128,.b=0};
								struct razer_rgb cb = {.r=0,.g=0,.b=128};
								struct razer_rgb cw = {.r=255,.g=255,.b=255};
								struct razer_pos old;
								old.x=pos.x;
								old.y=pos.y;
								convert_keycode_to_pos(event->code,&pos);							

								//double pangle=rad2deg(pos_angle_radians(&old,&pos));
								double pangle=pos_angle_radians(&old,&pos);
								//printf("pangle:%f\n",pangle);
								scroll_dir_y=-cos(pangle);
								scroll_dir_x=-sin(pangle);
								//printf("dir:%f,%f\n",scroll_dir_x,scroll_dir_y);

								keys->heatmap[pos.y][pos.x]+=1;//80;
								//if(keys->heatmap[pos.y][pos.x]>255)
								//	keys->heatmap[pos.y][pos.x]=255;

								//set_key_pos(keys,&pos,&color);
								//if(!(ev_count%6))
								//	clear_all(keys);
								if(actual_mode!=4)
								{
									if(ev_count%4==0)
									{
										set_keys_row(keys,pos.y,&cr);
										set_keys_column(keys,pos.x,&cr);
									}
									if(ev_count%4==1)
									{
										set_keys_row(keys,pos.y,&cg);
										set_keys_column(keys,pos.x,&cg);
									}
									if(ev_count%4==2)
									{
										set_keys_row(keys,pos.y,&cb);
										set_keys_column(keys,pos.x,&cb);
									}
									//if(ev_count%4==3)
									//{
										//set_keys_row(keys,pos.y,&cb);
										//set_keys_column(keys,pos.x,&cb);
										//draw_circle(keys,&pos,1,&cw);
										//draw_circle(keys,&pos,2,&cr);
										//draw_circle(keys,&pos,3,&cw);
										draw_ring(keys,&pos,3,&cw);
									//}
								}
								//update_keys(keys);
								ev_count++;
							}
							last_keycode = keycode;
						}
					}
				}
				//update_keys(keys);
			}
		}	
		update_keys(keys);
		update_sdl(keys,renderer,window,tex);
		last_event_count = ev_count;
	}
}
*/

/*
void create_sdl_window()
{
   	SDL_Init(SDL_INIT_VIDEO);      
	SDL_Window *sdl_window;
	SDL_Renderer *sdl_renderer;
	SDL_CreateWindowAndRenderer(22*32, 6*32, SDL_WINDOW_RESIZABLE, &sdl_window, &sdl_renderer);
	SDL_SetWindowTitle(sdl_window,"Razer Chroma Setup/Debug");
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(sdl_renderer);
	SDL_RenderPresent(sdl_renderer);
	SDL_Texture *sdl_texture = SDL_CreateTexture(sdl_renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,22,6);
	load_icons(sdl_renderer,"icons",sdl_icons);
}


void close_sdl_window()
{
  	SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
*/
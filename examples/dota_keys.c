#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "../lib/razer_chroma.h"

/*ignore unused parameters warning*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	struct razer_chroma *chroma = razer_open();
	if(!chroma)
		exit(1);
	razer_set_custom_mode(chroma);
	razer_clear_all(chroma->active_device->keys);
	razer_update_keys(chroma,chroma->active_device->keys);


	struct razer_rgb red = {.r=255,.g=0,.b=0}; //define a red color
	struct razer_rgb yellow = {.r=255,.g=255,.b=0}; //define a yellow color
	struct razer_rgb green = {.r=0,.g=255,.b=0}; //define a green color
	struct razer_rgb blue = {.r=0,.g=0,.b=255}; //define a blue color
	struct razer_rgb light_blue = {.r=0,.g=255,.b=255}; //define a light blue color

	struct razer_pos pos;

	char *abilities = "QWERDF";

	for(unsigned int i = 0;i<strlen(abilities);i++)
	{	
		razer_convert_ascii_to_pos(abilities[i],&pos);
		razer_set_key_pos(chroma->active_device->keys,&pos,&red);
	}

	char *groups = "1234567";

	for(unsigned int i = 0;i<strlen(groups);i++)
	{	
		razer_convert_ascii_to_pos(groups[i],&pos);
		razer_set_key_pos(chroma->active_device->keys,&pos,&yellow);
	}

	char *items = "YXCV";

	for(unsigned int i = 0;i<strlen(items);i++)
	{	
		razer_convert_ascii_to_pos(items[i],&pos);
		razer_set_key_pos(chroma->active_device->keys,&pos,&light_blue);
	}


	razer_convert_ascii_to_pos('B',&pos);
	razer_set_key_pos(chroma->active_device->keys,&pos,&green);

	razer_convert_ascii_to_pos('A',&pos);
	razer_set_key_pos(chroma->active_device->keys,&pos,&blue);

	razer_convert_ascii_to_pos('S',&pos);
	razer_set_key_pos(chroma->active_device->keys,&pos,&green);

	/*set the logo too*/
	pos.x = 20;
	pos.y = 0;
	razer_set_key_pos(chroma->active_device->keys,&pos,&green);


	razer_update_keys(chroma,chroma->active_device->keys);

 	razer_close(chroma);
}

#pragma GCC diagnostic pop

# razer_blackwidow_chroma_driver
A Linux driver for the Razer Blackwidow Chroma keyboard (supports all lighting modes) includes a daemon for advanced effects


Installation: 

 - execute install_driver.sh and reboot





How to create a standalone effect easily using the included library ?

First of all we need an idea what the effect shall do.

In this example i just setup the keyboard for a dota profile

First we need to setup the library:

	struct razer_chroma *chroma = razer_open();


To create an custom keyboard led layout we need to tell the library to activate the custom mode:

	razer_set_custom_mode(chroma);

If the keyboard was using the custom mode before the keys are still lit with the last color settings ,so let us clear it:

	razer_clear_all(chroma->keys);

To actually update the keyboard leds we need to razer_update (using the integrated keyboard led frame/keys struct):

	razer_update_keys(chroma,chroma->keys);

So now that we got a black keyboard we want to light some keys in different colors,
lets start with the abilities (Q,W,E,R,D,F):

	struct razer_rgb red = {.r=255,.g=0,.b=0}; //define a red color
	struct razer_rgb yellow = {.r=255,.g=255,.b=0}; //define a yellow color
	struct razer_rgb purple = {.r=255,.g=0,.b=255}; //define a purple color
	struct razer_rgb green = {.r=0,.g=255,.b=0}; //define a green color
	struct razer_rgb blue = {.r=0,.g=0,.b=255}; //define a blue color
	struct razer_rgb light_blue = {.r=0,.g=255,.b=255}; //define a light blue color
	
	struct razer_pos pos;

	char *abilities = "QWERDF";

	for(int i = 0;i<strlen(abilities);i++)
	{	
		razer_convert_ascii_to_pos(abilities[i],&pos);
		razer_set_key_pos(chroma->keys,&pos,&red);
	}

	char *groups = "1234567";

	for(int i = 0;i<strlen(groups);i++)
	{	
		razer_convert_ascii_to_pos(groups[i],&pos);
		razer_set_key_pos(chroma->keys,&pos,&yellow);
	}

	char *items = "YXCV";

	for(int i = 0;i<strlen(items);i++)
	{	
		razer_convert_ascii_to_pos(items[i],&pos);
		razer_set_key_pos(chroma->keys,&pos,&light_blue);
	}


	razer_convert_ascii_to_pos('B',&pos);
	razer_set_key_pos(chroma->keys,&pos,&green);

	razer_convert_ascii_to_pos('A',&pos);
	razer_set_key_pos(chroma->keys,&pos,&blue);

	razer_convert_ascii_to_pos('S',&pos);
	razer_set_key_pos(chroma->keys,&pos,&green);


Dont forget to update the keyboard with the new led color values:

	razer_update_keys(chroma,chroma->keys);


Freeing the library is just as easy:

 	razer_close(chroma);


To compile just type:

	gcc  -std=c99  dota_keys.c  -lrazer_chroma  -lm  -o dota_keys

After executing it you should now have a dota profile lighting up your keyboard.











 Additional Credits:

 - Various installation and makefile related fixes by Jordan King (manual merge)
 - Ubuntu file permission fixes by Carsten Teibes (pulled)
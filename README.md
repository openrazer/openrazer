# Razer Drivers

A collection of Linux drivers for the Razer devices, providing kernel drivers, DBus services and python bindings to interact with the DBus interface.

Website: [Here](http://pez2001.github.io/razer_chroma_drivers/)

**Before raising an issue** saying something doesn't work, read [this](https://github.com/pez2001/razer_chroma_drivers/wiki/Troubleshooting) Wiki page, try not to create new issues if one exists, reopen it.

## Support
### Keyboard Support:
 * Razer Blackwidow Classic *(all lighting modes)*
 * Razer Blackwidow Ultimate 2012 *(all lighting modes)*
 * Razer Blackwidow Ultimate 2013 *(all lighting modes)*
 * Razer Blackwidow Chroma *(all lighting modes)*
 * Razer Blackwidow Chroma Tournament Edition *(all lighting modes)*
 * Razer Blade Stealth *(all lighting modes)*
 * Razer Blackwidow Ultimate 2016 (all bar custom lighting)

### Mousemat Support:
 * Razer Firefly *(all lighting modes)*

### Mouse Support:
 * Razer Mamba *(all lighting modes)*
 * Razer Abyssus (all modes bar the refresh rate)

## Installation
[Here](https://github.com/pez2001/razer_chroma_drivers/wiki/Installation) is a page documenting the installation and uninstallation procedures.

## Applications

The following applications compliment and interact with this driver:

* [Polychromatic Controller](https://github.com/lah7/polychromatic-controller) - a graphical management tool and tray applet to managing Razer peripherals.
* [MarcoW](https://github.com/igorbb/MacroW) - a simple tool to record and play keyboard macros.


## Command Line Usage


 Have a look at the scripts directory.
 In the driver sub directory you will find the scripts to
 start the builtin keyboard effects.


 - Changing effect example

 :: the following command will create a render node for the effect with the unique id: 8 [fx_uid,name,description] ::

	razer_bcd_controller -C 8 "bars example" "new rendering node"

[Returns the render node uid of the created node]


:: to activate the render node with the unique id:2 execute the next command [rn_uid] (hint: the id was returned by the previous command) ::

	razer_bcd_controller -b 2

[You should now see 3 moving color bars (Red,Blue,Green)]


 - Customizing effects (setting of parameters) example

:: Getting the available parameters of an effect with the unique id: 2 [rn_uid] ::

 	razer_bcd_controller -F 2

 [Returns a json formatted list of the available effect parameters]

 in this case:

	{
 	"parameters_num" : 3 ,
 	"parameters_list": [
	{
 	"key": "Effect Counter Array",
 	"id" : 1 ,
 	"type" : 11 ,
 	"description": "Counter values(int array)" },
	{
 	"key": "Effect Direction Array",
 	"id" : 2 ,
 	"type" : 11 ,
 	"description": "Direction values(int array)" },
	{
 	"key": "Effect Colors Array",
 	"id" : 3 ,
 	"type" : 14 ,
 	"description": "Base colors(rgb array)" },
	]}

:: Setting a parameter for an effect with the unique id: 2 
   [rn_uid,parameter_index,
    array_index (use -1 if not an array),
    parameter_type,value(s) (enquote multiple values and use whitespaces to seperate)] ::	

	razer_bcd_controller -S 2 2 0 rgb "255 255 0"

[One bar should now have a yellow color]

:: Return to default effect :: 

	razer_bcd_controller -b 1

[You should now see the heatmap like default effect again]

 
 
 And take a look at the daemon and tests sub directories in scripts.
 The daemon uses dbus as its IPC mechanism, so you are not bound to shell scripts
 (someone may even write a Gui to control the daemon , maybe like the node editor in blender)


### Bash functions

In the file `/usr/share/razer_bcd/bash_keyboard_functions.sh` there are some functions used before and after the daemon is started/stopped. These functions bind and unbind the chroma to the kernel
driver. You can source the file and then run `bind_all`, this will attempt to bind chromas and skip any already binded. There is also a function called `unbind_all` which as you would
of guessed unbinds all chroma keyboards.






## Daemon IPC details

[... To be written ...]
 
 for a sneak peak take a look at:
 
 (bash interface using daemon_controller)
  - scripts/daemon/alert.sh
  - scripts/tests/test_daemon.sh
  - scripts/tests/test_glimmer.sh
 
 or simply call daemon_controller/razer_bcd_controller -h 
 for a command overview.

 (c api)
  - lib/razer_chroma_controller.h 
  - examples/controller_example.c



## Status of Code

 - **Driver :** Release Candidate
 - **Daemon :** Alpha
 - **Daemon Effects :** Release Candidate
 - **Daemon Controller :** Beta
 - **Libraries :** Beta
 - **Installer :** Beta
 - **Packages :** Beta

## First Steps Tutorial


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

So now that we got a black keyboard we want to light some keys in different colors

        struct razer_rgb red = {.r=255,.g=0,.b=0}; //define a red color
        struct razer_rgb yellow = {.r=255,.g=255,.b=0}; //define a yellow color
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

After executing it you should now have a dota profile lighting up your keyboard.(dont forget to sudo)
This is just a simple example using a ascii helper,if your profile needs to color function keys ,etc
you can set the key colors by manually setting the pos.






##Daemon effects tutorial


How to create an effect to be used in the daemon ?
Why not shoot for something crazy like a light blast originating from keys being pressed this time?
Its not that much different than writing a self-hosted effect.

[... To be written ...]






##Contributions


Any effect or tool you might want to contribute is welcome.
Please use your own source files to host your effects for merging.
FX setup scripts, bug fixes, feature requests, etc are also welcome.


## TODO


- support remaining effect handlers not called yet once
- key locking / automatically skip key on following frame changes 
  / manual overwrite still possible / catch in convience functions
- daemon,internal heatmap examples
- integrate `examples/dynamic` with daemon for passwordless GUI interface ?
- daemon to remember current effect and colours in use ?
- move remaining lib functions to razer_ namespace
- move all daemon types to daemon_ namespace
- split library into seperate source files (rgb,frames,hsl,drawing)
- free memory / fix leaks
- submit kernel patch ?
- customizable layout effect
- fft analyzer effect
- better multi device support
- configuration for daemon (mouse input device,chroma devices to be used,etc)
- add custom event sending via controller api
- add list loaded libs command (automatic dupe check too)
- add a minimal (custom mode only) daemon integrated libusb driver (windows/mac osx support)







## Additional Credits
 - Various installation and makefile related fixes by Jordan King (manual merge)
 - Ubuntu file permission fixes by Carsten Teibes (pulled)
 - Debugging help of Mosie1 with Linux Mint script bugs (testing)
 - Example Effect  : Dynamic by TheKiwi5000 (pulled)
 - Snake Example by James Shawver (pulled and edited slightly)
 - Identifying of missing speed parameter for the reactive mode by Oleg Finkelshteyn (implemented by maintainer)
   discoverying of a previous unknown reactive+wave mode.
   (call reactive script & wave script with none as parameter)
   support for the tournament edition(manual merge)
 - Modifications to dynamic example by Stephanie Sunshine (pulled)
 - Mamba Driver, Device quering functionality in the kernel, Deb packaging, DKMS support, shell scripting additions and ubuntu fixes by Terry Cain (pulled)
 - GUI Interface for BlackWidow Chroma Keyboards by Luke Horwell [(now migrated as Polychromatic Controller)](https://github.com/lah7/polychromatic-controller)
 - Ubuntu fixes by Brad Murmz (pulled)
 - Default Keyboard profiles by Mathieu Okuyama (manual merge)

## Donations (in Euros)

Goal 1 (66/66)  [Completed]: 

 All donations go towards buying a Razer Firefly Led-Mousepad (no driver written yet/easy to incorporate)

 - Reward:  Sound driven example effect(Spectrum based).

Stretch Goal 1 (166 of 166):
 
Buying a Razer Tartarus (also no driver written yet)

 - Reward:  Packaging for major distributions (debian based,redhat based,tar balls)

Stretch Goal 2 (213 of 333):
 Buying a Razer Deathstalker Utimate (no driver too)
 - Reward:  A gtk based GUI (simple , something to build upon further).

 - All goals contain the linux driver for the respective device as reward also.



Thank you for all donations i really appreciate it!



  - Luca Steeb 5
  - Klee Dienes 150
  - Josh Ventura 25
  - Trace McCabe 25
  - Anonymous 5
  - Luis Fernandes 3.5

  You can send your donations via PayPal to : feckelburger [at] gmx.net
  
  You can also send PayPal donations to terry@terrys-home.co.uk if you want :wink:

-----

The project is licensed under the GPL and is not affiliated with [Razer, Inc](http://www.razerzone.com/).

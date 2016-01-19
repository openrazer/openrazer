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
#include "razer_daemon_controller.h"




const char *dc_helpmsg_start = "Usage: %s [OPTIONS]... [COMMAND] [PARAMETERS]...\n\
Send commands to razer_bcd daemon.\n\
\n\
Commands:\n\
  -q    Close daemon\n\
  -c    Continue rendering\n\
  -p    Pause rendering\n\
  -C    Create rendering node\n\
           1. Parameter: Effect uid - sets effect render node uses\n\
           2. Parameter: device uid - unique id of device to use\n\
           3. Parameter: Name - sets the render nodes name\n\
           4. Parameter: Description - sets the render nodes description\n\
\n\
           For example: %s -C 1 \"My render node\" \"My description\"\n\
  -l    Load fx library\n\
           1. Parameter: Library filename\n\
  -f    Set fps rate\n\
           1. Parameter: fps rate\n\
  -g    Get fps rate\n\
           Returns: actual fps rate of daemon\n\
  -o    Set render node opacity\n\
           1. Parameter: render node uid - render node to set the opacity to\n\
           2. Parameter: opacity value - float value between 0.0 and 1.0\n\
  -O    Get render node opacity\n\
           1. Parameter: render node uid - render node to get the opacity of\n\
           Returns: opacity of render node as a float value between 0.0 and 1.0\n\
  -i    Is rendering paused ?\n\
           Returns: 0/1 running/paused\n\
  -x    Get fx list\n\
           Returns: fx list as json string\n\
  -X    Get render nodes list\n\
           Returns: render nodes list as json string\n\
  -R    Get rendering nodes list\n\
           1. Parameter: device uid - unique id of device to query\n\
           Returns: rendering nodes list as json string\n\
  -U    Get sub nodes list\n\
           1. Parameter: render node uid - render node to get the sub nodes of\n\
           Returns: sub nodes list as json string\n\
  -F    Get render node parameters list\n\
           1. Parameter: render node uid - render node to get the parameters of\n\
           Returns: render node parameter list as json string\n\
  -a    Get the actual render node uid connected to the framebuffer\n\
           Returns: uid of node\n\
  -t    Get the parent of a render node\n\
           1. Parameter: render node uid - render node to get the parent of\n\
           Returns: uid of parent node\n\
  -L    Set render node rendering time limit\n\
           1. Parameter: render node uid - render node to set the time limit\n\
           2. Parameter: time limit value - time span in ms\n\
  -b    Connect frame buffer to render node\n\
           1. Parameter: device uid - unique id of device to query\n\
           2. Parameter: render node uid - render node that gets connected to the frame buffer\n\
  -s    Add Sub-node to render node\n\
           1. Parameter: render node uid - render node the sub node should be added to\n\
           2. Parameter: sub node uid - sub node that gets added\n\
  -r    Connect input node to render nodes first input slot\n\
           1. Parameter: render node uid - render node the input node should be connected to\n\
           2. Parameter: input node uid - input node to connect\n\
  -n    Connect input node to render nodes second input slot\n\
           1. Parameter: render node uid - render node the input node should be connected to\n\
           2. Parameter: input node uid - input node to connect\n";
  const char *dc_helpmsg_end = "  -w    Get the next node of a render node\n\
           1. Parameter: render node uid - render node to get the next node of\n\
           Returns: uid of next node\n\
  -y    Set the next node of a render node\n\
           1. Parameter: render node uid - render node to set the next node of\n\
           2. Parameter: next node uid - next node to run after render node finished\n\
  -M    Get the move_linkage value of a render node\n\
           1. Parameter: render node uid - render node to get the move_linkage value of\n\
           Returns: uid of next node\n\
  -G    Set the move_linkage value of a render node\n\
           1. Parameter: render node uid - render node to get the next node of\n\
           2. Parameter: move_linkage - 0/1 activate/deactivate moving of framebuffer\n\
                         linkage of a render node\n\
  -A    Reset a render node\n\
           1. Parameter: render node uid - render node to reset\n\
  -P    Get the parameter of a render node\n\
           1. Parameter: render node uid - render node the parameter belongs to\n\
           2. Parameter: parameter index - index of parameter to get\n\
           3. Parameter: array index - if parameter is an array this index will be used (optional)\n\
           Returns: parameter as json\n\
  -S    Set the parameter of a render node\n\
           1. Parameter: render node uid - render node the parameter belongs to\n\
           2. Parameter: parameter index - index of parameter to set\n\
           3. Parameter: array index - if parameter is an array this index will be used (use -1 to skip)\n\
           4. Parameter: parameter type - type of value to be set (Int,Float,Rgb,String,etc)\n\
           5. Parameter: parameter value - value to set\n\
  -d    Disconnect frame buffer\n\
  -1    Spectrum Effect Mode\n\
  -2    Wave Effect Mode\n\
           1. Parameter: Direction, 0 = None, 1 = Right, 2 = Left\n\
  -3    Reactive Mode\n\
           1. Parameter: Speed (1-3)\n\
           2. Parameter: Red (0-255)\n\
           3. Parameter: Blue (0-255)\n\
           4. Parameter: Green (0-255)\n\
  -4    Breath Mode\n\
           1. Parameter: Random 1\n\n\
           1. Parameter: Red (0-255)\n\
           2. Parameter: Blue (0-255)\n\
           3. Parameter: Green (0-255)\n\
           4. Optional Parameter: Red 2 (0-255)\n\
           5. Optional Parameter: Blue 2 (0-255)\n\
           6. Optional Parameter: Green 2 (0-255)\n\
  -5    Static Mode\n\
           1. Parameter: Red (0-255)\n\
           2. Parameter: Blue (0-255)\n\
           3. Parameter: Green (0-255)\n\
  -6    None Mode\n\
  -7    Set keyboard brightness\n\
           1. Parameter: Brightness (0-255)\n\
  -8    Enable Macro Keys\n\
  -h    Display this help and exit\n\
\n\
Options:\n\
  -v    More verbose output\n\
\n\
	DBUS must be running on the system to communicate with daemon.\n\
\n\
      Report bugs to <pez2001@voyagerproject.de>.\n";

/* 1 spectrum mode
 * 2 wave mode
 * 3 reactive mode
 * 4 breath mode
 * 5 static mode
 * 6 none mode
 * 7 set brightness
 * 8 enable macro keys
 */

int verbose = 0;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	#ifndef USE_DBUS
		printf("You need to have dbus & dbus dev packages installed\n");
		return(1);
	#else
	char c;
	struct razer_daemon_controller *controller=NULL;
	if(!(controller=dc_open()))
	{
		printf("razer_bcd_controller: error initializing daemon controller\n");
		return(1);
	}
	
	int opts_given = 0;
	while((c=getopt(argc,argv,"hvVcpqlfoigatOLxbdsrnwyCMGPSXRUF12345678A")) != -1)
	{
		opts_given = 1;
		switch(c)
		{
			case '1':
				{
			    	// Spectrum Effect
			    	dc_set_spectrum_mode(controller);
				}
				break;
		  	case '2':
		  		{
			    	// Wave effect
			    	unsigned char direction = atoi(argv[optind++]);
				    dc_set_wave_mode(controller, direction);
				}
		  		break;
			case '3':
		  		{
		    		// Reactive Mode
		  			unsigned char speed = atoi(argv[optind++]);
		    		unsigned char red = atoi(argv[optind++]);
		    		unsigned char green = atoi(argv[optind++]);
		    		unsigned char blue = atoi(argv[optind++]);
				    dc_set_reactive_mode(controller, speed, red, green, blue);
				}
		  		break;
		  	case '4':
		  		{
		    		// Breath Mode
		    		unsigned char red = atoi(argv[optind++]);
		    		if(optind < argc)
		    		{
		    			unsigned char green = atoi(argv[optind++]);
		    			unsigned char blue = atoi(argv[optind++]);

		    			if(optind < argc)
		    			{
		    				unsigned char red2 = atoi(argv[optind++]);
		    				unsigned char green2 = atoi(argv[optind++]);
		    				unsigned char blue2 = atoi(argv[optind++]);
		    				dc_set_breath_mode(controller, 1, red, green, blue, red2, green2, blue2);
		    			} else
		    			{
		    				dc_set_breath_mode(controller, 1, red, green, blue, 0, 0, 0);
		    			}
		    		} else
		    		{
		    			dc_set_breath_mode(controller, 3, 0, 0, 0, 0, 0, 0);
		    		}
		  		}
		  		break;
		  	case '5':
		  		{
		    		// Static Mode
		    		unsigned char red = atoi(argv[optind++]);
		    		unsigned char green = atoi(argv[optind++]);
		    		unsigned char blue = atoi(argv[optind++]);
				    dc_set_static_mode(controller, red, green, blue);
		  		}
		  		break;
		  	case '6':
		  		{	
		    		// No effect mode
		    		dc_set_none_mode(controller);
				}
		  		break;
			case '7':
		  		{
		    		// Set brightness
		    		unsigned char brightness = atoi(argv[optind++]);
				    dc_set_keyboard_brightness(controller, brightness);
				}
		  		break;
		  	case '8':
		  		{
		    		// Enable macro keys
		    		dc_enable_macro_keys(controller);
				}
				break;
		  	case 'A':
		  		{
					int render_node_uid = atoi(argv[optind++]);
				    dc_render_node_reset(controller,render_node_uid);
				}
		  		break;
			case 'P':
				{
					int render_node_uid = atoi(argv[optind++]);
					int parameter_index = atoi(argv[optind++]);
					int array_index = -1;
					if(optind < argc)
						array_index = atoi(argv[optind++]);
					char *parameter_json = dc_render_node_parameter_get(controller,render_node_uid,parameter_index,array_index);
					if(verbose)
					{
						printf("sending get parameter value of render node: %d.%d.%d.\n",render_node_uid,parameter_index,array_index);
						printf("value: %s.\n",parameter_json);
					}
					else
						printf("%s",parameter_json);
					//free(parameter_json);
				}
				break;
			case 'S':
				{
					int render_node_uid = atoi(argv[optind++]);
					int parameter_index = atoi(argv[optind++]); // TODO switch to parameter uid someday
					//int parameter_uid = atoi(argv[optind++]);
					int	array_index = atoi(argv[optind++]);
					char *type = argv[optind++];
					char *value = argv[optind++];
					if(verbose)
						printf("sending set parameter value of render node: %d.%d.%d = [%s].\n",render_node_uid,parameter_index,array_index,value);
					dc_render_node_parameter_parsed_set(controller,render_node_uid,parameter_index,array_index,type,value);
				}
				break;
			case 'M':
				{
					int render_node_uid = atoi(argv[optind++]);
					int move_linkage = atoi(argv[optind]);
					if(verbose)
						printf("sending set move_linkage to %d for render node: %d command to daemon.\n",move_linkage,render_node_uid);
					dc_render_node_next_move_frame_buffer_linkage_set(controller,render_node_uid,move_linkage);
				}
				break;
			case 'G':
				{
					int render_node_uid = atoi(argv[optind++]);
					if(verbose)
					{
						printf("sending get move_linkage of render node: %d.\n",render_node_uid);
						printf("move linkage of render node: %d.\n",dc_render_node_next_move_frame_buffer_linkage_get(controller,render_node_uid));
					}
					else
						printf("%d",dc_render_node_next_move_frame_buffer_linkage_get(controller,render_node_uid));
				}
				break;
			case 'C':
				{
					int fx_uid = atoi(argv[optind++]);
					int device_uid = atoi(argv[optind++]);
					char *node_name = argv[optind++];
					char *node_description = argv[optind++];
					if(verbose)
					{
						printf("sending create render node command to daemon.\n");
						printf("new render node uid: %d.\n",dc_render_node_create(controller,fx_uid,device_uid,node_name,node_description));
					}
					else
						printf("%d",dc_render_node_create(controller,fx_uid,device_uid,node_name,node_description));

				}
				break;
			case 'y':
				{
					int render_node_uid = atoi(argv[optind++]);
					int next_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending set next node : %d for render node: %d command to daemon.\n",next_node_uid,render_node_uid);
					dc_render_node_next_set(controller,render_node_uid,next_node_uid);
				}
				break;
			case 'w':
				{
					int render_node_uid = atoi(argv[optind++]);
					if(verbose)
					{
						printf("sending get next node of render node: %d.\n",render_node_uid);
						printf("next render node: %d.\n",dc_render_node_next_get(controller,render_node_uid));
					}
					else
						printf("%d",dc_render_node_next_get(controller,render_node_uid));
				}
				break;
			case 'r':
				{
					int render_node_uid = atoi(argv[optind++]);
					int input_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending connect node : %d to render nodes: %d first input command to daemon.\n",input_node_uid,render_node_uid);
					dc_render_node_input_connect(controller,render_node_uid,input_node_uid);
				}
				break;
			case 'n':
				{
					int render_node_uid = atoi(argv[optind++]);
					int input_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending connect node : %d to render nodes: %d second input command to daemon.\n",input_node_uid,render_node_uid);
					dc_render_node_second_input_connect(controller,render_node_uid,input_node_uid);
				}
				break;
			case 's':
				{
					int render_node_uid = atoi(argv[optind++]);
					int sub_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending add sub node: %d to render node: %d command to daemon.\n",sub_node_uid,render_node_uid);
					dc_render_node_sub_add(controller,render_node_uid,sub_node_uid);
				}
				break;
			case 'b':
				{
					int device_uid = atoi(argv[optind++]);
					int render_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending connect frame buffer %d to render node: %d command to daemon.\n",device_uid,render_node_uid);
					dc_frame_buffer_connect(controller,device_uid,render_node_uid);
				}
				break;
			case 'd':
				{
					int device_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending disconnect frame buffer command to daemon.\n");
					dc_frame_buffer_disconnect(controller,device_uid);
				}
				break;
			case 'x':
				{
					char *list = dc_fx_list(controller);
					if(verbose)
					{	
						printf("sending get effects list command to daemon.\n");
						printf("daemon fx list:\n%s.\n",list);
					}
					else
						printf("%s",list);
					free(list);
				}
				break;
			case 'X':
				{
					char *list = dc_render_nodes_list(controller);
					if(verbose)
					{	
						printf("sending get render nodes list command to daemon.\n");
						printf("daemon render nodes list:\n%s.\n",list);
					}
					else
						printf("%s",list);
					free(list);
				}
				break;
			case 'R':
				{
					int device_uid = atoi(argv[optind]);
					char *list = dc_rendering_nodes_list(controller,device_uid);
					if(verbose)
					{	
						printf("sending get rendering nodes list command to daemon.\n");
						printf("daemon %d rendering nodes list:\n%s.\n",device_uid,list);
					}
					else
						printf("%s",list);
					free(list);
				}
				break;
			case 'U':
				{
					int render_node_uid = atoi(argv[optind++]);
					char *list = dc_sub_nodes_list(controller,render_node_uid);
					if(verbose)
					{	
						printf("sending get sub nodes list command to daemon.\n");
						printf("daemon sub nodes list:\n%s.\n",list);
					}
					else
						printf("%s",list);
					free(list);
				}
				break;
			case 'F':
				{
					int render_node_uid = atoi(argv[optind++]);
					char *list = dc_render_node_parameters_list(controller,render_node_uid);
					if(verbose)
					{	
						printf("sending get render node parameters list command to daemon.\n");
						printf("daemon render node parameters list:\n%s.\n",list);
					}
					else
						printf("%s",list);
					free(list);
				}
				break;
			case 'L':
				{
					int render_node_uid = atoi(argv[optind++]);
					int time_limit_ms = atoi(argv[optind]);
					if(verbose)
						printf("sending render node limit render time command to daemon.\n");
					dc_render_node_limit_render_time_ms_set(controller,render_node_uid,time_limit_ms);
				}
				break;
			case 't':
				{
					int render_node_uid = atoi(argv[optind]);
					if(verbose)
					{
						printf("sending get parent render node command to daemon.\n");
						printf("render node parent:%d.\n",dc_render_node_parent_get(controller,render_node_uid));
					}
					else
						printf("%d",dc_render_node_parent_get(controller,render_node_uid));
				}
				break;
			case 'a':
				{
					int device_uid = atoi(argv[optind]);
					if(verbose)
					{
						printf("sending get framebuffer connected render node command to daemon.\n");
						printf("daemon is running node:%d.\n",dc_frame_buffer_get(controller,device_uid));
					}
					else
						printf("%d",dc_frame_buffer_get(controller,device_uid));
				}
				break;
			case 'g':
				if(verbose)
				{
					printf("sending get fps command to daemon.\n");
					printf("daemon is running at %d fps.\n",dc_fps_get(controller));
				}
				else
					printf("%d",dc_fps_get(controller));
				break;
			case 'i':
				if(verbose)
				{
					printf("sending is paused command to daemon.\n");
					int paused = dc_is_paused(controller);
					if(paused)
						printf("daemon is paused.\n");
					else
						printf("daemon is running.\n");
				}
				else
					printf("%d",dc_is_paused(controller));
				break;
			case 'o':
				{
					int render_node_uid = atoi(argv[optind++]);
					double opacity = atof(argv[optind]);
					if(verbose)
						printf("sending set opacity for render node: %d command to daemon: %f.\n",render_node_uid,opacity);
					dc_render_node_opacity_set(controller,render_node_uid,opacity);
				}
				break;
			case 'O':
				{
					int render_node_uid = atoi(argv[optind]);
					if(verbose)
					{
						printf("sending get opacity for render node: %d command to daemon.\n",render_node_uid);
						printf("render node opacity is to: %f.\n",dc_render_node_opacity_get(controller,render_node_uid));
					}
					else
						printf("%f",dc_render_node_opacity_get(controller,render_node_uid));
				}
				break;
			case 'f':
				{
					int fps = atoi(argv[optind]);
					if(verbose)
						printf("sending set fps command to daemon: %d.\n",fps);
					dc_fps_set(controller,fps);
				}
				break;
			case 'l':
				if(verbose)
					printf("sending load fx library command to daemon: library to load: \"%s\".\n",argv[optind]);
				dc_load_fx_lib(controller,argv[optind]);
				break;
			case 'c':
				if(verbose)
					printf("sending continue command to daemon.\n");
				dc_continue(controller);
				break;
			case 'p':
				if(verbose)
					printf("sending pause command to daemon.\n");
				dc_pause(controller);
				break;
			case 'q':
				if(verbose)
					printf("sending quit command to daemon.\n");
				dc_quit(controller);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'h':
				printf("Razer blackwidow chroma daemon controller\n");
				printf(dc_helpmsg_start,argv[0],argv[0]);
				printf(dc_helpmsg_end,argv[0],argv[0]);
				return(0);
			case 'V':
				printf("razer_bcd daemon controller %d.%d (build %d)\n",MAJOR_VERSION,MINOR_VERSION);
				return(0);
			case '?':
				if(optopt == 'c')
					printf("Option -%c requires an argument.\n",optopt);
				else if(isprint(optopt))
					printf("Unknown option `-%c'.\n",optopt);
				else
					printf("Unknown option character `\\x%x'.\n",optopt);
				opts_given = 0;
				return(1);
			default:
				opts_given = 0;
				abort();
		}
	}
	dc_close(controller);
	if(!opts_given)
	{
		printf("Razer blackwidow chroma daemon controller\n");
		printf(dc_helpmsg_start,argv[0],argv[0]);
		printf(dc_helpmsg_end,argv[0],argv[0]);
	}		
	#endif
	return(0);
}

#pragma GCC diagnostic pop


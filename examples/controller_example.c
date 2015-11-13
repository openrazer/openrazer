#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "getopt.h"
#include "ctype.h"


#include "../lib/razer_chroma.h"
#include "../lib/razer_chroma_controller.h"
//#include "../daemon/razer_daemon.h"





#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	#ifndef USE_DBUS
		printf("You need to have dbus & dbus dev packages installed\n");
		return(1);
	#else



	struct razer_daemon_controller *controller=NULL;
	if(!(controller=dc_open()))
	{
		printf("controller_example: error initializing daemon controller example\n");
		return(1);
	}

	char *mixer_fx_lib = "daemon/fx/pez2001_mixer_debug.so";
	printf("sending load fx library command to daemon: library to load: \"%s\".\n",mixer_fx_lib);
	dc_load_fx_lib(controller,mixer_fx_lib);

	int wv_fx_uid = 6;//uid of the light_blast fx
	char *wv_node_name = "Wave Effect";
	char *wv_node_description = "overlay";
	printf("sending create render node command to daemon.\n");
	int wv_node_uid = dc_render_node_create(controller,wv_fx_uid,wv_node_name,wv_node_description);
	printf("new render node uid (wv): %d.\n",wv_node_uid);

	int lb_fx_uid = 11;//uid of the light_blast fx
	char *lb_node_name = "Light Blast Effect";
	char *lb_node_description = "heatmap alike";
	printf("sending create render node command to daemon.\n");
	int lb_node_uid = dc_render_node_create(controller,lb_fx_uid,lb_node_name,lb_node_description);
	printf("new render node uid (lb): %d.\n",lb_node_uid);

	int mx_fx_uid = 18;//uid of the light_blast fx
	char *mx_node_name = "Mixer";
	char *mx_node_description = "effects mixer";
	printf("sending create render node command to daemon.\n");
	int mx_node_uid = dc_render_node_create(controller,mx_fx_uid,mx_node_name,mx_node_description);
	printf("new render node uid (mx): %d.\n",mx_node_uid);


	printf("sending connect node : %d to render nodes: %d first input command to daemon.\n",wv_node_uid,mx_node_uid);
	dc_render_node_input_connect(controller,mx_node_uid,wv_node_uid);

	printf("sending connect node : %d to render nodes: %d first input command to daemon.\n",lb_node_uid,mx_node_uid);
	dc_render_node_input_connect(controller,mx_node_uid,lb_node_uid);


	int fps = 8;
	printf("sending set fps command to daemon: %d.\n",fps);
	dc_fps_set(controller,fps);

	double opacity = 0.2f;
	printf("sending set opacity for render node: %d command to daemon: %f.\n",mx_node_uid,opacity);
	dc_render_node_opacity_set(controller,mx_node_uid,opacity);


	printf("sending connect frame buffer to render node: %d command to daemon.\n",mx_node_uid);
	dc_frame_buffer_connect(controller,mx_node_uid);




	/*	
	int render_node_uid = atoi(argv[optind++]);
	int parameter_uid = atoi(argv[optind++]);
	int array_index = -1;
	array_index = atoi(argv[optind++]);
	char *parameter_json = dc_render_node_parameter_get(controller,render_node_uid,parameter_uid,array_index);
	printf("sending get parameter value of render node: %d.%d.%d.\n",render_node_uid,parameter_uid,array_index);
	printf("value: %s.\n",parameter_json);

	int render_node_uid = atoi(argv[optind++]);
	int move_linkage = atoi(argv[optind]);
	printf("sending set move_linkage to %d for render node: %d command to daemon.\n",move_linkage,render_node_uid);
	dc_render_node_next_move_frame_buffer_linkage_set(controller,render_node_uid,move_linkage);

	int render_node_uid = atoi(argv[optind++]);
	printf("sending get move_linkage of render node: %d.\n",render_node_uid);
	printf("move linkage of render node: %d.\n",dc_render_node_next_move_frame_buffer_linkage_get(controller,render_node_uid));




	int render_node_uid = atoi(argv[optind++]);
	int next_node_uid = atoi(argv[optind]);
	printf("sending set next node : %d for render node: %d command to daemon.\n",next_node_uid,render_node_uid);
	dc_render_node_next_set(controller,render_node_uid,next_node_uid);


	int render_node_uid = atoi(argv[optind++]);
	printf("sending get next node of render node: %d.\n",render_node_uid);
	printf("next render node: %d.\n",dc_render_node_next_get(controller,render_node_uid));



	int render_node_uid = atoi(argv[optind++]);
	int input_node_uid = atoi(argv[optind]);
	printf("sending connect node : %d to render nodes: %d second input command to daemon.\n",input_node_uid,render_node_uid);
	dc_render_node_second_input_connect(controller,render_node_uid,input_node_uid);


	int render_node_uid = atoi(argv[optind++]);
	int sub_node_uid = atoi(argv[optind]);
	printf("sending add sub node: %d to render node: %d command to daemon.\n",sub_node_uid,render_node_uid);
	dc_render_node_sub_add(controller,render_node_uid,sub_node_uid);




	printf("sending disconnect frame buffer command to daemon.\n");
	dc_frame_buffer_disconnect(controller);

	char *list = dc_fx_list(controller);
	printf("sending get effects list command to daemon.\n");
	printf("daemon fx list:\n%s.\n",list);
	free(list);

	int render_node_uid = atoi(argv[optind++]);
	int time_limit_ms = atoi(argv[optind]);
	printf("sending render node limit render time command to daemon.\n");
	dc_render_node_limit_render_time_ms_set(controller,render_node_uid,time_limit_ms);

	int render_node_uid = atoi(argv[optind]);
	printf("sending get parent render node command to daemon.\n");
	printf("render node parent:%d.\n",dc_render_node_parent_get(controller,render_node_uid));

	printf("sending get framebuffer connected render node command to daemon.\n");
	printf("daemon is running node:%d.\n",dc_frame_buffer_get(controller));

	printf("sending get fps command to daemon.\n");
	printf("daemon is running at %d fps.\n",dc_fps_get(controller));

	printf("sending is paused command to daemon.\n");
	int paused = dc_is_paused(controller);
	if(paused)
		printf("daemon is paused.\n");
	else
		printf("daemon is running.\n");


	int render_node_uid = atoi(argv[optind]);
	printf("sending get opacity for render node: %d command to daemon.\n",render_node_uid);
	printf("render node opacity is to: %f.\n",dc_render_node_opacity_get(controller,render_node_uid));




	printf("sending continue command to daemon.\n");
	dc_continue(controller);
	printf("sending pause command to daemon.\n");
	dc_pause(controller);
	printf("sending quit command to daemon.\n");
	dc_quit(controller);
	*/
	dc_close(controller);
	return(0);
	#endif
}

#pragma GCC diagnostic pop


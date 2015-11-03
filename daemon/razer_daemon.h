#ifndef _RAZER_DAEMON_H_
#define _RAZER_DAEMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <dlfcn.h>
#include <time.h>

#include "../lib/razer_chroma.h"

#ifdef USE_DBUS
	#include <dbus/dbus.h>
#endif


//struct razer_daemon;
//struct razer_fx_render_node;


#define RAZER_COMPOSE_MODE_MIX 1

struct razer_daemon 
{
	struct razer_chroma *chroma;
	int running;
	int is_paused;
	int fps;
	struct razer_fx_render_node *render_node;
	list *render_nodes;//struct razer_rx_render_node
	int is_render_nodes_dirty;
	struct razer_fx_render_node *return_render_node;
	struct razer_rgb_frame *frame_buffer;
	int frame_buffer_linked_uid;//need to know if i have to free it or it is a link to another buffer
	int libs_uid;
	list *libs; //struct daemon_lib
	int effects_uid;
	list *effects;//struct razer_effect
	int fx_render_nodes_uid;
	list *fx_render_nodes;//struct razer_rx_render_node
	#ifdef USE_DBUS
		DBusConnection *dbus;
	#endif
};

void daemon_kill(struct razer_daemon *daemon,char *error_message);
void daemon_compute_render_nodes(struct razer_daemon *daemon);


int daemon_input_event_handler(struct razer_chroma *chroma,struct razer_chroma_event *event);





#include "razer_daemon_types.h"
#include "razer_daemon_parameters.h"
#include "razer_daemon_effects.h"
#include "razer_daemon_render_nodes.h"
#include "razer_daemon_libraries.h"
#include "razer_daemon_dbus.h"

#endif
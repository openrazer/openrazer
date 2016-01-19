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
#include <getopt.h>

#include "../lib/razer_chroma.h"

#ifdef USE_DBUS
	#include <dbus/dbus.h>
#endif


//struct razer_daemon;
//struct razer_fx_render_node;


#define RAZER_COMPOSE_MODE_MIX 1


//struct to hold device specific data
//uses struct razer_chroma_device->tag
struct razer_daemon_device_data 
{
	int id;
	struct razer_daemon *daemon;
	struct razer_chroma_device *chroma_device;
	list *render_nodes;//struct razer_rx_render_node
	int is_render_nodes_dirty;
	struct razer_fx_render_node *default_render_node;
	struct razer_fx_render_node *return_render_node;
	struct razer_rgb_frame *frame_buffer;
	int frame_buffer_linked_uid;//need to know if i have to free it or it is a link to another buffer

};

struct razer_daemon
{
	struct razer_chroma *chroma;
	int running;
	int is_paused;
	int fps;
	//list *devices;//struct razer_daemon_device
	//a list of global computation effects
	list *computation_render_nodes;//struct razer_rx_render_node
	int libs_uid;
	list *libs; //struct daemon_lib
	int effects_uid;
	list *effects;//struct razer_effect
	int fx_render_nodes_uid;
	list *fx_render_nodes;//struct razer_rx_render_node
	int devices_uid;
	#ifdef USE_DBUS
		DBusConnection *dbus;
	#endif
};

struct daemon_options
{
	int daemonize;
	int verbose;
	char *pid_file;
	char *keyboard_input_file;
	char *mouse_input_file;
};

void daemon_kill(struct razer_daemon *daemon,char *error_message);
void daemon_compute_render_nodes(struct razer_chroma_device *device);
int daemon_event_handler(struct razer_chroma *chroma,struct razer_chroma_event *event);
struct razer_chroma_device *daemon_get_device(struct razer_daemon *daemon,int uid);




#include "razer_daemon_types.h"
#include "razer_daemon_parameters.h"
#include "razer_daemon_effects.h"
#include "razer_daemon_render_nodes.h"
#include "razer_daemon_libraries.h"
#include "razer_daemon_dbus.h"

#endif

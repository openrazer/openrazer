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
#ifndef _RAZER_CHROMA_CONTROLLER_H_
#define _RAZER_CHROMA_CONTROLLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include "getopt.h"
#include "ctype.h"


#include "../lib/razer_chroma.h"
#include "../daemon/razer_daemon.h"

#ifdef USE_DBUS
	#include <dbus/dbus.h>
#endif


struct razer_daemon_controller 
{
	int running;
	#ifdef USE_DBUS
		DBusConnection *dbus;
		DBusPendingCall *pending;
	#endif
};



#ifdef USE_DBUS
	int dc_dbus_error_check(char*message,DBusError *error);
	int dc_dbus_open(struct razer_daemon_controller *controller);
	void dc_dbus_close(struct razer_daemon_controller *controller);
#endif

struct razer_daemon_controller *dc_open(void);
void dc_close(struct razer_daemon_controller *controller);
void dc_error_close(struct razer_daemon_controller *controller,char *message);
void dc_quit(struct razer_daemon_controller *controller);
void dc_continue(struct razer_daemon_controller *controller);
void dc_pause(struct razer_daemon_controller *controller);
int dc_render_node_create(struct razer_daemon_controller *controller,int effect_uid,int device_uid,char *name,char *description);
void dc_render_node_reset(struct razer_daemon_controller *controller,int render_node_uid);//TODO give back result
void dc_default_render_node_set(struct razer_daemon_controller *controller,int device_uid,int render_node_uid);
char *dc_render_node_parameter_get(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid,int array_index);
int dc_render_node_parameter_parsed_set(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid,int array_index,char *type,char *value_string);
void dc_render_node_parameter_set(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid,int array_index,int type,unsigned long long value);
float dc_render_node_opacity_get(struct razer_daemon_controller *controller,int render_node_uid);
void dc_render_node_opacity_set(struct razer_daemon_controller *controller,int render_node_uid,float opacity);
void dc_render_node_input_connect(struct razer_daemon_controller *controller,int render_node_uid,int input_render_node_uid);
void dc_render_node_second_input_connect(struct razer_daemon_controller *controller,int render_node_uid,int second_input_render_node_uid);
int dc_render_node_next_get(struct razer_daemon_controller *controller,int render_node_uid);
void dc_render_node_next_set(struct razer_daemon_controller *controller,int render_node_uid,int next_render_node_uid);
int dc_render_node_next_move_frame_buffer_linkage_get(struct razer_daemon_controller *controller,int render_node_uid);
void dc_render_node_next_move_frame_buffer_linkage_set(struct razer_daemon_controller *controller,int render_node_uid,int move_linkage);
int dc_render_node_parent_get(struct razer_daemon_controller *controller,int render_node_uid);
void dc_render_node_limit_render_time_ms_set(struct razer_daemon_controller *controller,int render_node_uid,int limit_ms);
void dc_render_node_sub_add(struct razer_daemon_controller *controller,int render_node_uid,int sub_render_node_uid);
void dc_frame_buffer_connect(struct razer_daemon_controller *controller,int device_uid,int render_node_uid);
void dc_frame_buffer_disconnect(struct razer_daemon_controller *controller,int device_uid);
char *dc_fx_list(struct razer_daemon_controller *controller);
char *dc_render_nodes_list(struct razer_daemon_controller *controller);
char *dc_rendering_nodes_list(struct razer_daemon_controller *controller,int device_uid);
char *dc_sub_nodes_list(struct razer_daemon_controller *controller,int render_node_uid);
char *dc_render_node_parameters_list(struct razer_daemon_controller *controller,int render_node_uid);
int dc_is_paused(struct razer_daemon_controller *controller);
void dc_fps_set(struct razer_daemon_controller *controller,int fps);
int dc_fps_get(struct razer_daemon_controller *controller);
int dc_frame_buffer_get(struct razer_daemon_controller *controller,int device_uid);
void dc_load_fx_lib(struct razer_daemon_controller *controller,char *fx_pathname);

void dc_set_spectrum_mode(struct razer_daemon_controller *controller);
void dc_set_wave_mode(struct razer_daemon_controller *controller, unsigned char direction);
void dc_set_reactive_mode(struct razer_daemon_controller *controller, unsigned char speed, unsigned char red, unsigned char green, unsigned char blue);
void dc_set_breath_mode(struct razer_daemon_controller *controller, unsigned char breath_type, unsigned char red, unsigned char green, unsigned char blue,
						unsigned char red2, unsigned char green2, unsigned char blue2);
void dc_set_static_mode(struct razer_daemon_controller *controller, unsigned char red, unsigned char green, unsigned char blue); // TODO refactor to use one method instead of 3
void dc_set_none_mode(struct razer_daemon_controller *controller);
void dc_set_keyboard_brightness(struct razer_daemon_controller *controller, unsigned char brightness);
void dc_enable_macro_keys(struct razer_daemon_controller *controller);

#endif

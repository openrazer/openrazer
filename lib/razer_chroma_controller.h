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
int dc_render_node_create(struct razer_daemon_controller *controller,int effect_uid,char *name,char *description);
void dc_render_node_set(struct razer_daemon_controller *controller,int render_node_uid);
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
void dc_frame_buffer_connect(struct razer_daemon_controller *controller,int render_node_uid);
void dc_frame_buffer_disconnect(struct razer_daemon_controller *controller);
char *dc_fx_list(struct razer_daemon_controller *controller);
char *dc_render_nodes_list(struct razer_daemon_controller *controller);
char *dc_rendering_nodes_list(struct razer_daemon_controller *controller);
char *dc_sub_nodes_list(struct razer_daemon_controller *controller,int render_node_uid);
char *dc_render_node_parameters_list(struct razer_daemon_controller *controller,int render_node_uid);
int dc_is_paused(struct razer_daemon_controller *controller);
void dc_fps_set(struct razer_daemon_controller *controller,int fps);
int dc_fps_get(struct razer_daemon_controller *controller);
int dc_frame_buffer_get(struct razer_daemon_controller *controller);
void dc_load_fx_lib(struct razer_daemon_controller *controller,char *fx_pathname);

#endif
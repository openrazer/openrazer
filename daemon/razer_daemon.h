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

#include "../lib/razer_chroma.h"

#include <dbus/dbus.h>



struct razer_effect;

typedef int (*razer_effect_open)(struct razer_fx_render_node *render);
typedef int (*razer_effect_close)(struct razer_fx_render_node *render);
typedef int (*razer_effect_update)(struct razer_fx_render_node *render);
typedef int (*razer_effect_key_event)(struct razer_fx_render_node *render);
typedef int (*razer_effect_dbus_event)(struct razer_fx_render_node *render);

typedef struct razer_effect *(*razer_effect_init)(struct razer_daemon *daemon);

struct razer_daemon;

#define RAZER_PARAMETER_TYPE_STRING 1
#define RAZER_PARAMETER_TYPE_INT 2
#define RAZER_PARAMETER_TYPE_FLOAT 3
#define RAZER_PARAMETER_TYPE_RGB 4


struct razer_parameter
{
	char *key;
	void *value;
	int type;
};

struct razer_effect
{
	int id;
	char *name;
	char *description;
	razer_effect_open open;
	razer_effect_close close;
	razer_effect_update update;
	razer_effect_key_event key_event;
	razer_effect_dbus_event dbus_event;
	struct razer_parameter **parameters;
};

struct razer_fx_render_node 
{
	int compose_mode;
	struct razer_rgb_frame input;
	struct razer_rgb_frame output;
	struct razer_daemon *daemon;
	struct razer_effect *effect;
	struct razer_parameter **parameters;
	struct razer_fx_render_node *parent;
	int subs_num;
	struct razer_fx_render_node **subs;
};

struct razer_daemon 
{
	struct razer_chroma *chroma;
	DBusConnection *dbus;
	int running;
	int fps;
	//struct razer_keys *keys;
	int effects_num;
	//struct razer_keys_locks locks;
	struct razer_effect **effects;
};





#endif
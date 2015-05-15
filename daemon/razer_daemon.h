#ifndef _RAZER_DAEMON_H_
#define _RAZER_DAEMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "../lib/razer_chroma.h"


#define RAZER_MAX_EFFECTS 200

struct razer_effect;

typedef int (*razer_effect_open_handler)(struct razer_effect *effect);
typedef int (*razer_effect_close_handler)(struct razer_effect *effect);
typedef int (*razer_effect_update_handler)(struct razer_effect *effect);

typedef struct razer_effect *(*razer_effect_create_handler)();

struct razer_effect
{
	int id;
	int compose_mode;
	struct razer_rgb_frame input;
	struct razer_rgb_frame output;
	//struct razer_keys_locks locks;
	struct razer_keys *keys;
	razer_effect_open_handler *open;
	razer_effect_close_handler *close;
	razer_effect_update_handler *update;
	void *parameters;
	void *tag;
};


struct razer_daemon 
{
	struct razer_chroma *chroma;
	//struct razer_keys *keys;
	int effects_num;
	struct razer_effect *effects[RAZER_MAX_EFFECTS];
};





#endif
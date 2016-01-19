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
 #include "razer_daemon.h"


char *daemon_effect_to_json(struct razer_effect *effect, int final)
{
	char *effect_json = str_CreateEmpty();
	effect_json = str_CatFree(effect_json,"\n {\n \"name\": \"");
	effect_json = str_CatFree(effect_json,effect->name);
	effect_json = str_CatFree(effect_json,"\" ,\n");
	effect_json = str_CatFree(effect_json," \"id\" : ");
	char *id_string = str_FromLong(effect->id);
	effect_json = str_CatFree(effect_json,id_string);
	effect_json = str_CatFree(effect_json," ,\n");
	free(id_string);
	effect_json = str_CatFree(effect_json," \"description\": \"");
	effect_json = str_CatFree(effect_json,effect->description);
	effect_json = str_CatFree(effect_json,"\" ,\n \"parameters_num\" : ");
	char *parameters_num_string = str_FromLong(effect->parameters->num);
	effect_json = str_CatFree(effect_json,parameters_num_string);
	effect_json = str_CatFree(effect_json," ,\n");
	free(parameters_num_string);
	effect_json = str_CatFree(effect_json," \"parameters\" :  [\n");
	for(int i = 0;i<effect->parameters->num;i++)
	{
		struct razer_parameter *parameter = effect->parameters->items[i];
		char *parameter_json;
		if(i == effect->parameters->num - 1)
		{
			parameter_json= daemon_parameter_to_json(parameter, 1);
		} else {
			parameter_json= daemon_parameter_to_json(parameter, 0);
		}

		effect_json = str_CatFree(effect_json,parameter_json);
		free(parameter_json);
	}
	if(final)
	{
		effect_json = str_CatFree(effect_json,"] }\n");
	} else {
		effect_json = str_CatFree(effect_json,"] },\n");
	}

	return(effect_json);
}


//needed to be able to use different parameter sets for multiple copies of the same effect in der render_nodes chain
struct razer_effect *daemon_create_effect_instance(struct razer_daemon *daemon,struct razer_effect *lib_effect)
{
	//create an instance of a base effect housed in external libs
	//create own instances of each razer_parameter included
	//return the instance
	struct razer_effect *instance = daemon_create_effect();
	instance->id = daemon->effects_uid++;
	instance->name = str_Copy(lib_effect->name);
	instance->description = str_Copy(lib_effect->description);
	instance->fps = lib_effect->fps;
	instance->open = lib_effect->open;
	instance->close = lib_effect->close;
	instance->update = lib_effect->update;
	instance->reset = lib_effect->reset;
	instance->handle_event = lib_effect->handle_event;
	instance->dbus_event = lib_effect->dbus_event;
	for(int i=0;i<lib_effect->parameters->num;i++)
	{
		daemon_effect_add_parameter(instance,daemon_copy_parameter(list_Get(lib_effect->parameters,i)));
	}
	return(instance);
}

int daemon_register_effect(struct razer_daemon *daemon,struct razer_effect *effect)
{
	list_Push(daemon->effects,effect);
	effect->id = daemon->effects_uid++;
	//#ifdef USE_DEBUGGING
	//	printf("registered effect: %s with uid: %d\n",effect->name,effect->id);
	//#endif
	return(effect->id);
}

/*
int daemon_unregister_effect(struct razer_daemon *daemon,struct razer_effect *effect)
{
	//remove effect
	//fill gap if there is one left by effect
	//realloc
}
*/

struct razer_effect *daemon_get_effect(struct razer_daemon *daemon,int uid)
{
	for(int i = 0;i<list_GetLen(daemon->effects);i++)
	{
		struct razer_effect *effect = list_Get(daemon->effects,i);
		if(effect->id == uid)
			return(effect);
	}
	return(NULL);
}

struct razer_effect *daemon_create_effect(void)
{
	struct razer_effect *effect = (struct razer_effect*)malloc(sizeof(struct razer_effect));
	effect->parameters = list_Create(0,0);
	effect->parameters_uid = 1;
	effect->name = NULL;
	effect->description = NULL;
	effect->open = NULL;
	effect->close = NULL;
	effect->update = NULL;
	effect->handle_event = NULL;
	effect->dbus_event = NULL;
	effect->fps = 1;
	effect->input_usage_mask = 0;
	effect->effect_class = 0;
	effect->id = 0;
	return(effect);
}

void daemon_free_effect(struct razer_effect *effect)
{
	daemon_free_parameters(effect->parameters);
	free(effect);
}

/*
int daemon_effect_fire_parameter_changed(struct razer_effect *effect)
{
	if(!effect)
		return(-1);
	if(!effect->parameter_changed)
		return(-1);
	int ret = effect->parameter_changed(render_node);
	return(ret);
}
*/

int daemon_effect_add_parameter(struct razer_effect *effect,struct razer_parameter *parameter)
{
	list_Push(effect->parameters,parameter);
	parameter->id = effect->parameters_uid++;
	return(parameter->id);
}

struct razer_parameter *daemon_effect_get_parameter_by_index(struct razer_effect *effect,int index)
{
	return(list_Get(effect->parameters,index));
}

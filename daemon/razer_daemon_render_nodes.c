#include "razer_daemon.h"

char *daemon_render_node_to_json(struct razer_fx_render_node *render_node)
{
	char *rn_json = str_CreateEmpty();
	rn_json = str_CatFree(rn_json,"\n {\n");
	rn_json = str_CatFree(rn_json," \"id\" : ");
	char *id_string = str_FromLong(render_node->id);
	rn_json = str_CatFree(rn_json,id_string);
	rn_json = str_CatFree(rn_json," ,\n");
	free(id_string);
	rn_json = str_CatFree(rn_json," \"name\": \"");
	rn_json = str_CatFree(rn_json,render_node->name);
	rn_json = str_CatFree(rn_json,"\" ,\ndescription\": \"");
	rn_json = str_CatFree(rn_json,render_node->description);
	/*effect_json = str_CatFree(effect_json,"\" ,\n \"subs_num\" : ");
	char *parameters_num_string = str_FromLong(effect->parameters->num);
	effect_json = str_CatFree(effect_json,parameters_num_string);
	effect_json = str_CatFree(effect_json," ,\n");
	free(parameters_num_string);
	effect_json = str_CatFree(effect_json," \"parameters\" :  [\n");
	for(int i = 0;i<effect->parameters->num;i++)
	{
		struct razer_parameter *parameter = effect->parameters->items[i];
		char *parameter_json = daemon_parameter_to_json(parameter);
		effect_json = str_CatFree(effect_json,parameter_json);
		free(parameter_json);
	}*/
	rn_json = str_CatFree(rn_json,"\"\n },\n");
	return(rn_json);
}

int daemon_register_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	list_Push(daemon->fx_render_nodes,render_node);
	render_node->id = daemon->fx_render_nodes_uid++;
	return(render_node->id);
}

struct razer_fx_render_node *daemon_create_render_node(struct razer_daemon *daemon,struct razer_effect *effect,int input_render_node_uid,int second_input_render_node_uid,int output_render_node_uid,char *name,char *description)
{
	struct razer_fx_render_node *render_node = (struct razer_fx_render_node*)malloc(sizeof(struct razer_fx_render_node));
	render_node->daemon = daemon;
	//render_node->effect = effect;
	if(effect)
		render_node->effect = daemon_create_effect_instance(daemon,effect);
	else
		render_node->effect = NULL;
	render_node->opacity = 1.0f;
	if(input_render_node_uid == -1)
	{
		struct razer_rgb_frame *iframe = razer_create_rgb_frame();
		render_node->input_frame = iframe;
		render_node->input_frame_linked_uid = -1;
	}
	else if(input_render_node_uid == 0) //set input to daemon output buffer
	{
		render_node->input_frame = daemon->frame_buffer;
		render_node->input_frame_linked_uid = 0;
	}
	else
	{
		struct razer_fx_render_node *rn = daemon_get_render_node(daemon,input_render_node_uid);
		render_node->input_frame = rn->output_frame;
		render_node->input_frame_linked_uid = input_render_node_uid;
	}

	if(second_input_render_node_uid == -1)
	{
		struct razer_rgb_frame *siframe = razer_create_rgb_frame();
		render_node->second_input_frame = siframe;
		render_node->second_input_frame_linked_uid = -1;
	}
	else if(second_input_render_node_uid == 0) //set input to daemon output buffer
	{
		render_node->second_input_frame = daemon->frame_buffer;
		render_node->second_input_frame_linked_uid = 0;
	}
	else
	{
		struct razer_fx_render_node *srn = daemon_get_render_node(daemon,second_input_render_node_uid);
		render_node->second_input_frame = srn->output_frame;
		render_node->second_input_frame_linked_uid = second_input_render_node_uid;
	}

	if(output_render_node_uid == -1)
	{
		struct razer_rgb_frame *oframe = razer_create_rgb_frame();
		render_node->output_frame = oframe;
		render_node->output_frame_linked_uid = -1;
	}
	else if(output_render_node_uid == 0) //set input to daemon output buffer
	{
		render_node->output_frame = daemon->frame_buffer;
		render_node->output_frame_linked_uid = 0;
	}
	/*else //not used
	{
		struct razer_fx_render_node *orn = daemon_get_render_node(daemon,output_render_node_uid);
		render_node->output_frame = orn->output_frame;
		render_node->output_frame_linked_uid = output_render_node_uid;
	}*/

	render_node->description = str_Copy(description);
	render_node->name = str_Copy(name);
	//render_node->fps = daemon->fps;
	render_node->compose_mode = RAZER_COMPOSE_MODE_MIX;
	render_node->next = NULL;
	render_node->parent = NULL;
	//render_node->parameters = NULL;
	//render_node->parameters_num = 0;
	render_node->subs = list_Create(0,0);
	render_node->start_ticks = 0;
	render_node->running = 0;//set to 1 with first update
	render_node->limit_render_time_ms = 0;
	render_node->move_frame_buffer_linkage_to_next = 1;
	//render_node->continue_chain=1;
	render_node->loop_count = -1;
	return(render_node);
}


struct razer_fx_render_node *daemon_get_render_node(struct razer_daemon *daemon,int uid)
{
	for(int i = 0;i<list_GetLen(daemon->fx_render_nodes);i++)
	{
		struct razer_fx_render_node *render_node = list_Get(daemon->fx_render_nodes,i);
		if(render_node->id == uid)
			return(render_node);
	}
	return(NULL);
}

void daemon_render_node_add_sub(struct razer_fx_render_node *render_node,struct razer_fx_render_node *sub_node)
{
	list_Push(render_node->subs,sub_node);
	sub_node->parent = render_node;
}



void daemon_connect_frame_buffer(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(daemon->frame_buffer_linked_uid != 0) //unlink old render node first
	{
		struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
		old_rn->output_frame = razer_create_rgb_frame();
		old_rn->output_frame_linked_uid = -1;
	}
	if(render_node->output_frame_linked_uid == -1)
		razer_free_rgb_frame(render_node->output_frame);
	render_node->output_frame = daemon->frame_buffer;
	daemon->frame_buffer_linked_uid = render_node->id;
	daemon->fps = render_node->effect->fps;
	daemon->is_render_nodes_dirty = 1;
}

void daemon_disconnect_frame_buffer(struct razer_daemon *daemon)
{
	if(daemon->frame_buffer_linked_uid != 0) //unlink old render node first
	{
		struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
		old_rn->output_frame = razer_create_rgb_frame();
		old_rn->output_frame_linked_uid = -1;
	}
	//if(render_node->output_frame_linked_uid == -1)
	//	razer_free_rgb_frame(render_node->output_frame);
	//daemon->frame_buffer = razer_create_rgb_frame();
	daemon->frame_buffer_linked_uid = -1;
	daemon->is_render_nodes_dirty = 1;
}

void daemon_connect_input(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node)
{
	//if(render_node->input_frame_linked_uid != 0) //unlink old render node first
	//{
	//	struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
	//	old_rn->output_frame = daemon_create_rgb_frame();
	//	old_rn->output_frame_linked_uid = -1;
	//}
	if(render_node->input_frame_linked_uid == -1)
		razer_free_rgb_frame(render_node->input_frame);
	render_node->input_frame = input_node->output_frame;
	render_node->input_frame_linked_uid = input_node->id;
	daemon->is_render_nodes_dirty = 1;
}

void daemon_connect_second_input(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node)
{
	//if(render_node->input_frame_linked_uid != 0) //unlink old render node first
	//{
	//	struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
	//	old_rn->output_frame = daemon_create_rgb_frame();
	//	old_rn->output_frame_linked_uid = -1;
	//}
	if(render_node->second_input_frame_linked_uid == -1)
		razer_free_rgb_frame(render_node->second_input_frame);
	render_node->second_input_frame = input_node->output_frame;
	render_node->second_input_frame_linked_uid = input_node->id;
	daemon->is_render_nodes_dirty = 1;
}

void daemon_set_default_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	daemon->render_node = render_node;
}

int daemon_has_render_node_reached_render_limit(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(render_node->limit_render_time_ms)
	{
		//printf("checking render_limit %d : %u > %u + %u (%u)\n",render_node->id,daemon->chroma->update_ms,render_node->start_ticks,render_node->limit_render_time_ms,render_node->start_ticks + render_node->limit_render_time_ms);
		if(daemon->chroma->update_ms > (render_node->start_ticks + render_node->limit_render_time_ms))
		{
			#ifdef USE_DEBUGGING
				printf("\nreached render limit for node: %d\n",render_node->id);
			#endif
			return(1);
		}
	}
	return(0);
}

int daemon_update_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(!render_node || !render_node->effect)
		return(-1);

	if(!render_node->start_ticks)
	{
		render_node->start_ticks = razer_get_ticks();
		render_node->running  = 1;
		for(int i=0;i<list_GetLen(render_node->subs);i++)
		{
			struct razer_fx_render_node *sub = list_Get(render_node->subs,i);
			sub->start_ticks = 0;
			sub->running  = 1;
		}
	}
	if(!render_node->running || daemon_has_render_node_reached_render_limit(daemon,render_node))
		return(0);
	if(list_GetLen(render_node->subs))
	{
		/*#ifdef USE_DEBUGGING
			printf("## has compute nodes ##");
		#endif*/
		for(int i=0;i<list_GetLen(render_node->subs);i++)
		{
			struct razer_fx_render_node *sub = list_Get(render_node->subs,i);
			if(!sub->effect)
				continue;
			if(sub->effect->class&=2)
				continue;//only execute compute effects
			if(!sub->start_ticks)
			{
				sub->start_ticks = razer_get_ticks();
				sub->running  = 1;
			}
			if(!sub->running)
				continue;
			int sub_ret = daemon_update_render_node(daemon,sub);
			if(!sub_ret || daemon_has_render_node_reached_render_limit(daemon,sub) || !sub->running)
			{
				if(sub->next)
				{
					list_Set(render_node->subs,i,sub->next);
					sub->next->parent = render_node;
					sub->next->start_ticks = 0; 
				}
				sub->running = 0;
				//return(0);
			}
		}
	}
	if(!render_node->effect->update)
		return(-1);
	int ret = render_node->effect->update(render_node);
	return(ret);
}

int daemon_input_event_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,struct razer_chroma_event *event)
{
	if(!render_node || !render_node->effect)
		return(-1);

	if(!render_node->start_ticks)
	{
		render_node->start_ticks = razer_get_ticks();
		render_node->running  = 1;
	}
	if(!render_node->running || daemon_has_render_node_reached_render_limit(daemon,render_node))
		return(0);
	if(list_GetLen(render_node->subs))
	{
		for(int i=0;i<list_GetLen(render_node->subs);i++)
		{
			struct razer_fx_render_node *sub = list_Get(render_node->subs,i);
			if(!sub->start_ticks)
			{
				sub->start_ticks = razer_get_ticks();
				sub->running  = 1;
			}
			if(!sub->running)
				continue;
			int sub_ret = daemon_input_event_render_node(daemon,sub,event);
			if(!sub_ret || daemon_has_render_node_reached_render_limit(daemon,sub) || !sub->running)
			{
				if(sub->next)
				{
					list_Set(render_node->subs,i,sub->next);
					sub->next->parent = render_node;
					sub->next->start_ticks = 0; 
				}
				sub->running = 0;
				//return(0);
			}
		}
	}
	if(!render_node->effect->input_event)
		return(-1);
	int ret = render_node->effect->input_event(render_node,event);
	return(ret);
}

#include "pez2001_mixer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

struct razer_effect *effect_mix = NULL;

int effect_mix_update(struct razer_fx_render_node *render)
{
	int x,y;
	#ifdef USE_DEBUGGING
		printf(" (Mixer::Basic.%d ## opacity:%f / %d,%d)",render->id,render->opacity,render->input_frame_linked_uid,render->second_input_frame_linked_uid);
	#endif
	//render->opacity = 0.5f;
	for(x=0;x<22;x++)
		for(y=0;y<6;y++)
		{
			rgb_mix_into(&render->output_frame->rows[y].column[x],&render->input_frame->rows[y].column[x],&render->second_input_frame->rows[y].column[x],render->opacity);
			render->output_frame->update_mask |= 1<<y;
		}
	return(1);
}

struct razer_effect *effect_null = NULL;

int effect_null_update(struct razer_fx_render_node *render)
{
	#ifdef USE_DEBUGGING
		printf(" (Compute::Null.%d ## )",render->id);
	#endif
	return(1);
}

struct razer_effect *effect_wait = NULL;

int effect_wait_update(struct razer_fx_render_node *render)
{
	#ifdef USE_DEBUGGING
		printf(" (Compute::Wait_update.%d ## )",render->id);
	#endif
	return(1);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int effect_wait_input_event(struct razer_fx_render_node *render,struct razer_chroma_event *event)
{
	#ifdef USE_DEBUGGING
		printf(" (Compute::Wait_event.%d ## )",render->id);
	#endif
	if(event->type == RAZER_CHROMA_EVENT_TYPE_KEYBOARD && event->sub_type)
		return(0);
	return(1);
}

#pragma GCC diagnostic pop


struct razer_effect *effect_transition = NULL;

int effect_transition_update(struct razer_fx_render_node *render)
{
	int length_ms = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	unsigned long start = render->start_ticks;
	unsigned long end = start + length_ms;
	unsigned long ticks_left = end - render->daemon->chroma->update_ms;
	float opacity = 0.0f;
	if(dir == 1)
		opacity = (float)ticks_left / (float)length_ms;
	else
		opacity = 1.0f - ((float)ticks_left / (float)length_ms);
	#ifdef USE_DEBUGGING
		printf(" (Compute::Transition.%d ## length_ms:%d,dir:%d,opacity:%f)",render->id,length_ms,dir,opacity);
	#endif
	if(end<render->daemon->chroma->update_ms)
	{
		if(dir == 1)
		{
			dir = -1;
			if(render->parent) //compute effects should only be added as sub so this should be always fine
				render->parent->opacity = 0.0f;
		}
		else
		{
			dir = 1;
			if(render->parent) //compute effects should only be added as sub so this should be always fine
				render->parent->opacity = 1.0f;
		}
		daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
		return(0);
	}
	if(render->parent) //compute effects should only be added as sub so this should be always fine
		render->parent->opacity = opacity;
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
	return(1);
}

struct razer_effect *effect_random_col = NULL;
struct razer_rgb effect_random_col_dst_rgb = {.r=0,.g=0,.b=0};
struct razer_rgb effect_random_col_src_rgb = {.r=0,.g=0,.b=0};

int effect_random_col_update(struct razer_fx_render_node *render)
{
	int length_ms = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int par_index = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	if(par_index == -1)
		return(0);
	int randomize_now = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	struct razer_rgb *dst_color = daemon_get_parameter_rgb(daemon_effect_get_parameter_by_index(render->effect,3));
	struct razer_rgb *src_color = daemon_get_parameter_rgb(daemon_effect_get_parameter_by_index(render->effect,4));
	if(randomize_now)
	{
		struct razer_rgb *org_color = NULL;
		if(render->parent) 
		{
			org_color = daemon_get_parameter_rgb(daemon_effect_get_parameter_by_index(render->parent->effect,par_index));
		}
		if(org_color)
		{
			src_color->r = org_color->r;
			src_color->g = org_color->g;
			src_color->b = org_color->b;
		}

		dst_color->r = random() % 256;
		dst_color->g = random() % 256;
		dst_color->b = random() % 256;
		daemon_set_parameter_rgb(daemon_effect_get_parameter_by_index(render->effect,3),dst_color);	
		daemon_set_parameter_rgb(daemon_effect_get_parameter_by_index(render->effect,4),src_color);	
		randomize_now = 0;
	}
	unsigned long start = render->start_ticks;
	unsigned long end = start + length_ms;
	unsigned long ticks_left = end - render->daemon->chroma->update_ms;
	float trans = 0.0f;
	trans = (float)ticks_left / (float)length_ms;
	if(end<render->daemon->chroma->update_ms)
	{
		//#ifdef USE_DEBUGGING
		//	printf("\n(Compute::RandomCol.%d ## finished)\n",render->id);
		//#endif
		daemon_set_parameter_rgb(daemon_effect_get_parameter_by_index(render->parent->effect,par_index),dst_color);	
		randomize_now = 1;
		daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),randomize_now);	
		return(0);
	}
	else
	{
		#ifdef USE_DEBUGGING
			printf(" (Compute::RandomCol.%d ## length_ms:%d,trans_color:%d,%d,%d,trans:%f)",render->id,length_ms,dst_color->r,dst_color->g,dst_color->b,trans);
		#endif
		if(render->parent) 
		{
			struct razer_rgb trans_color;
			//trans_color.r=dst_color->r*trans;
			rgb_mix_into(&trans_color,dst_color,src_color,trans);
			daemon_set_parameter_rgb(daemon_effect_get_parameter_by_index(render->parent->effect,par_index),&trans_color);	
		}
	}


	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),randomize_now);	
	return(1);
}


#pragma GCC diagnostic pop

struct razer_effect *effect_glimmer = NULL;

int effect_glimmer_update(struct razer_fx_render_node *render)
{
	int x,y;
	#ifdef USE_DEBUGGING
		printf(" (Mixer::Glimmer.%d ## opacity:%f / %d,%d)",render->id,render->opacity,render->input_frame_linked_uid,render->second_input_frame_linked_uid);
	#endif
	//render->opacity = 0.5f;
	for(x=0;x<22;x++)
		for(y=0;y<6;y++)
		{
			float pixel_opacity = render->opacity + ((((float)(random()%1000))/1000.0f)-(render->opacity*0.5f));
			//float pixel_opacity = ((((float)(random()%1000))/1000.0f));

			rgb_mix_into(&render->output_frame->rows[y].column[x],&render->input_frame->rows[y].column[x],&render->second_input_frame->rows[y].column[x],pixel_opacity);
			render->output_frame->update_mask |= 1<<y;
		}
	return(1);
}



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

void fx_init(struct razer_daemon *daemon)
{
	srand(time(NULL));
	struct razer_parameter *parameter = NULL;
	effect_transition = daemon_create_effect();
	effect_transition->update = effect_transition_update;
	effect_transition->name = "Slow Opacity Transition";
	effect_transition->description = "First compute only effect";
	effect_transition->fps = 20;
	effect_transition->class = 2;
	effect_transition->input_usage_mask = RAZER_EFFECT_NO_INPUT_USED;
	//parameter = daemon_create_parameter_int("End Counter","End of animation (Integer)",44);//TODO refactor to daemon_add_effect_parameter_int(effect,key,desc,value)
	//daemon_add_parameter(effect_transition->parameters,parameter);	
	parameter = daemon_create_parameter_int("Effect Length","Time effect lasts in ms(INT)",2000);
	daemon_effect_add_parameter(effect_transition,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect_transition,parameter);	

	int effect_transition_uid = daemon_register_effect(daemon,effect_transition);
	#ifdef USE_DEBUGGING
		printf("registered compute effect: %s (uid:%d)\n",effect_transition->name,effect_transition->id);
	#endif

	effect_mix = daemon_create_effect();
	effect_mix->update = effect_mix_update;
	effect_mix->name = "Default Mixer";
	effect_mix->description = "Standard effect mixer";
	effect_mix->fps = 20;
	effect_mix->class = 1;
	effect_mix->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED | RAZER_EFFECT_SECOND_INPUT_USED;
	int effect_mix_uid = daemon_register_effect(daemon,effect_mix);
	#ifdef USE_DEBUGGING
		printf("registered mix effect: %s (uid:%d)\n",effect_mix->name,effect_mix->id);
	#endif

	effect_null = daemon_create_effect();
	effect_null->update = effect_null_update;
	effect_null->name = "Empty Compute Node";
	effect_null->description = "Does nothing";
	effect_null->fps = 1;
	effect_null->class = 1;
	effect_null->input_usage_mask = RAZER_EFFECT_NO_INPUT_USED;
	int effect_null_uid = daemon_register_effect(daemon,effect_null);
	#ifdef USE_DEBUGGING
		printf("registered compute effect: %s (uid:%d)\n",effect_null->name,effect_null->id);
	#endif

	effect_wait = daemon_create_effect();
	//effect_wait->update = effect_wait_update;
	effect_wait->input_event = effect_wait_input_event;
	effect_wait->name = "Wait For Key Compute Node";
	effect_wait->description = "Waits for a key and returns 0 ,it does nothing else";
	effect_wait->fps = 1;
	effect_wait->class = 1;
	effect_wait->input_usage_mask = RAZER_EFFECT_NO_INPUT_USED;
	int effect_wait_uid = daemon_register_effect(daemon,effect_wait);
	#ifdef USE_DEBUGGING
		printf("registered compute effect: %s (uid:%d)\n",effect_wait->name,effect_wait->id);
	#endif

	effect_random_col = daemon_create_effect();
	effect_random_col->update = effect_random_col_update;
	effect_random_col->name = "Randomize Color Parameter Compute Node";
	effect_random_col->description = "Randomizes a color parameter of his parent (able to transition slowly)";
	effect_random_col->fps = 1;
	effect_random_col->class = 1;
	effect_random_col->input_usage_mask = RAZER_EFFECT_NO_INPUT_USED;
	parameter = daemon_create_parameter_int("Effect Length","Time transition lasts in ms(INT)",2000);
	daemon_effect_add_parameter(effect_random_col,parameter);	
	parameter = daemon_create_parameter_int("Parameter Index","Parent color parameter index to randomize(INT)",-1);
	daemon_effect_add_parameter(effect_random_col,parameter);	
	parameter = daemon_create_parameter_int("Randomize Now","Set to 1 to randomize the color(INT)",1);
	daemon_effect_add_parameter(effect_random_col,parameter);	
	parameter = daemon_create_parameter_rgb("Transition Destination Color","Randomized color store(RGB)",&effect_random_col_dst_rgb);
	daemon_effect_add_parameter(effect_random_col,parameter);	
	parameter = daemon_create_parameter_rgb("Transition Start Color","Internal color store(RGB)",&effect_random_col_src_rgb);
	daemon_effect_add_parameter(effect_random_col,parameter);	
	int effect_random_col_uid = daemon_register_effect(daemon,effect_random_col);
	#ifdef USE_DEBUGGING
		printf("registered compute effect: %s (uid:%d)\n",effect_random_col->name,effect_random_col->id);
	#endif

	effect_glimmer = daemon_create_effect();
	effect_glimmer->update = effect_glimmer_update;
	effect_glimmer->name = "Glimming Mixer";
	effect_glimmer->description = "Glimming colors switching between input effects";
	effect_glimmer->fps = 20;
	effect_glimmer->class = 1;
	effect_glimmer->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED | RAZER_EFFECT_SECOND_INPUT_USED;
	int effect_glimmer_uid = daemon_register_effect(daemon,effect_glimmer);
	#ifdef USE_DEBUGGING
		printf("registered mix effect: %s (uid:%d)\n",effect_glimmer->name,effect_glimmer->id);
	#endif


}

#pragma GCC diagnostic pop


void fx_shutdown(struct razer_daemon *daemon)
{
	daemon_unregister_effect(daemon,effect_transition);
	daemon_free_parameters(effect_transition->parameters);
	daemon_free_effect(effect_transition);

	daemon_unregister_effect(daemon,effect_mix);
	daemon_free_parameters(effect_mix->parameters);
	daemon_free_effect(effect_mix);

	daemon_unregister_effect(daemon,effect_null);
	daemon_free_parameters(effect_null->parameters);
	daemon_free_effect(effect_null);

	daemon_unregister_effect(daemon,effect_wait);
	daemon_free_parameters(effect_wait->parameters);
	daemon_free_effect(effect_wait);

	daemon_unregister_effect(daemon,effect_glimmer);
	daemon_free_parameters(effect_glimmer->parameters);
	daemon_free_effect(effect_glimmer);
}

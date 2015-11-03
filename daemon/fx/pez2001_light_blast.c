#include "../razer_daemon.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

int effect_key_ringbuffer_size = 20;

struct razer_effect *effect = NULL;
struct razer_int_array *effect_keystrokes = NULL;

int effect_update(struct razer_fx_render_node *render)
{
	//int length_ms = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,0));
	int dir = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1));
	//int key_ring_buffer_index = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	struct razer_int_array *keystrokes = daemon_get_parameter_int_array(daemon_effect_get_parameter_by_index(render->effect,3));
	//unsigned long start = render->start_ticks;
	//unsigned long end = start + length_ms;
	//unsigned long ticks_left = end - render->daemon->chroma->update_ms;
	int x,y;
	struct razer_pos key_pos[effect_key_ringbuffer_size];
	struct razer_rgb col;
	for(int i =0;i<effect_key_ringbuffer_size;i++)
	{
		if(keystrokes->values[i]!=0)
			razer_convert_keycode_to_pos(keystrokes->values[i],&key_pos[i]);
		else
		{
			key_pos[i].x = -1;
			key_pos[i].y = -1;
		}
	}
	#ifdef USE_DEBUGGING
		printf(" (Blast.%d ## opacity:%f / %d,%d)",render->id,render->opacity,render->input_frame_linked_uid,render->second_input_frame_linked_uid);
	#endif
	//render->opacity = 0.5f;
	float kdist = 0.0f;
	for(x=0;x<22;x++)
		for(y=0;y<6;y++)
		{
			//float dist = 0.0f;
			float dist = 0.0f;
			int knum = 0;
			for(int i = 0;i<effect_key_ringbuffer_size;i++)
			{
				//for every pos != -1,-1
				if(key_pos[i].x != -1 && key_pos[i].y != -1)
				{
					knum++;
					int dx = x - key_pos[i].x;
					int dy = y - key_pos[i].y;
					if(dx || dy)
						kdist = 2.4f / sqrt((dx*dx)+(dy*dy));
					else
						kdist = 2.4f;
					if(kdist>0.1f)
						dist += kdist;
					//if(kdist<dist)
					//	dist =kdist;
					//printf("dist(%d,%d : %d,%d):%f\n",x,y,key_pos[i].x,key_pos[i].y,kdist);
				}
				/*else
				{
					knum++;
					dist+=21.0f;
				}*/
			}
			//dist = 1.0f - ( dist / (((float)knum)*21.0f))*0.03f;
			//dist = dist / (((float)knum)*21.0f);
			//dist = dist / 210.0f;
			//dist = 1.0f-(2.0f / dist);
			//dist = (30.0f / dist);
			//dist = 1.0f-(dist / 20.0f);
			//dist = dist / 7.0f;
			dist = dist / 14.0f;
			//printf("dist(%d,%d):%f\n",x,y,dist);
			rgb_from_hue(dist,0.3f,0.0f,&col);

			//col.r = (unsigned char)(col_max.r * dist);
			rgb_mix_into(&render->output_frame->rows[y].column[x],&render->input_frame->rows[y].column[x],&col,render->opacity);//*render->opacity  //&render->second_input_frame->rows[y].column[x]
			render->output_frame->update_mask |= 1<<y;
		}
	daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
	return(1);
}
int effect_input_event(struct razer_fx_render_node *render,struct razer_chroma_event *event)
{
	if(event->type != RAZER_CHROMA_EVENT_TYPE_KEYBOARD)
		return(1);
	int key_ring_buffer_index = daemon_get_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2));
	struct razer_int_array *keystrokes = daemon_get_parameter_int_array(daemon_effect_get_parameter_by_index(render->effect,3));
	#ifdef USE_DEBUGGING
		printf(" (Compute::KeyRingBuffer.%d ## )",render->id);
	#endif
	if(event->sub_type)
	{
		if(key_ring_buffer_index==effect_key_ringbuffer_size)
			key_ring_buffer_index = 0;
		//daemon_set_parameter_int_array(daemon_effect_get_parameter_by_index(render->effect,3),keystrokes);	
		keystrokes->values[key_ring_buffer_index++] = (long)event->value;
		daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),key_ring_buffer_index);	
		return(0);
	}
	//daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,1),dir);	
	//daemon_set_parameter_int(daemon_effect_get_parameter_by_index(render->effect,2),key_ring_buffer_index);	
	return(1);
}


#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

void fx_init(struct razer_daemon *daemon)
{
	srand(time(NULL));
	struct razer_parameter *parameter = NULL;
	effect_keystrokes = daemon_create_int_array(effect_key_ringbuffer_size,1);//storage for 10 keystrokes with a fixed size
	effect = daemon_create_effect();
	effect->update = effect_update;
	effect->input_event = effect_input_event;
	effect->name = "Lightblaster";
	effect->description = "Light field influenced by last keystrokes";
	effect->fps = 20;
	effect->class = 1;
	effect->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;
	parameter = daemon_create_parameter_int("Effect Length","Time effect lasts in ms(INT)",2000);
	daemon_effect_add_parameter(effect,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction","Effect direction value(INT)",1);
	daemon_effect_add_parameter(effect,parameter);	
	parameter = daemon_create_parameter_int("Effect Keystroke Index","Effect keystroke ringbuffer index value(INT)",0);
	daemon_effect_add_parameter(effect,parameter);	
	parameter = daemon_create_parameter_int_array("Effect Keystrokes","Effect last keystrokes storage array(INT)",effect_keystrokes);
	daemon_effect_add_parameter(effect,parameter);	
	int effect_uid = daemon_register_effect(daemon,effect);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect->name,effect->id);
	#endif

}

#pragma GCC diagnostic pop


void fx_shutdown(struct razer_daemon *daemon)
{
	free(effect_keystrokes);//TODO move to daemon method
	daemon_unregister_effect(daemon,effect);
	daemon_free_parameters(effect->parameters);
	daemon_free_effect(effect);
}

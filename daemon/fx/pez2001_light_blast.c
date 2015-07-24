#include "pez2001_mixer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

struct razer_effect *effect = NULL;

int effect_update(struct razer_fx_render_node *render)
{
	int x,y;
	#ifdef USE_DEBUGGING
		printf(" (Blast.%d ## opacity:%f / %d,%d)",render->id,render->opacity,render->input_frame_linked_uid,render->second_input_frame_linked_uid);
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
int effect_key_event(struct razer_fx_render_node *render,int keycode,int pressed)
{
	#ifdef USE_DEBUGGING
		printf(" (Compute::Wait_event.%d ## )",render->id);
	#endif
	if(pressed)
		return(0);
	return(1);
}


#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

void fx_init(struct razer_daemon *daemon)
{
	srand(time(NULL));
	struct razer_parameter *parameter = NULL;

	effect = daemon_create_effect();
	effect->update = effect_update;
	effect->key_event = effect_key_event;
	effect->name = "Default Mixer";
	effect->description = "Standard effect mixer";
	effect->fps = 20;
	effect->class = 1;
	effect->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED | RAZER_EFFECT_SECOND_INPUT_USED;
	parameter = daemon_create_parameter_int("Effect Length","Time effect lasts in ms(INT)",2000);
	daemon_add_parameter(effect->parameters,parameter);	
	parameter = daemon_create_parameter_int("Effect Direction","Effect direction value(INT)",1);
	daemon_add_parameter(effect->parameters,parameter);	
	int effect_mix_uid = daemon_register_effect(daemon,effect);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect->name,effect->id);
	#endif

}

#pragma GCC diagnostic pop


void fx_shutdown(struct razer_daemon *daemon)
{
	daemon_unregister_effect(daemon,effect);
	daemon_free_parameters(&effect->parameters);
	daemon_free_effect(&effect);
}

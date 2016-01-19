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
 #include "../razer_daemon.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"


struct razer_effect *effect = NULL;

int effect_update(struct razer_fx_render_node *render)
{
	float percentage = daemon_get_parameter_float(daemon_effect_get_parameter_by_index(render->effect,0));
	int x,y;
	struct razer_rgb col;
	#ifdef USE_DEBUGGING
		printf(" (Bar.%d ## %%:%f)",render->id,percentage);
	#endif

	int xmax = (int)((21.0f / 100.0f) * percentage);
	for(x=0;x<render->device->columns_num;x++)
		for(y=0;y<render->device->rows_num;y++)
		{
			float dist = 0.0f;
			if(x-xmax <= 0)
				dist = 1.0f;
			else
			{
				dist = (21.0f / (x-xmax))/21.0f;
			}
			rgb_from_hue(dist,0.3f,0.0f,&col);
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&col,render->opacity);//*render->opacity  //&render->second_input_frame->rows[y]->column[x]
			render->output_frame->update_mask |= 1<<y;
		}
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
	effect->name = "Progress Bar #1";
	effect->description = "Progress bar to display a percentage";
	effect->fps = 15;
	effect->effect_class = 1;
	effect->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;
	parameter = daemon_create_parameter_float("Effect Percentage","Percentage to display(FLOAT)",0.0f);
	daemon_effect_add_parameter(effect,parameter);	
	int effect_uid = daemon_register_effect(daemon,effect);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect->name,effect->id);
	#endif

}

#pragma GCC diagnostic pop


void fx_shutdown(struct razer_daemon *daemon)
{
	daemon_unregister_effect(daemon,effect);
	daemon_free_parameters(effect->parameters);
	daemon_free_effect(effect);
}

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
#include "input_example.h"

#define keys_max 10
int keys_history_index = 0;
int keys_history[keys_max];//ring buffer
int running = 1;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

void effect(struct razer_chroma *chroma)
{
	int r,g,b;
	int rnd=random();
	int rnd2=random();
	int rnd3=random();
	int count = 0;
	int count_dir = 1;
	float s = 0.1f;
	int x,y;
	struct razer_rgb col;
	struct razer_pos pos;
	col.r = 255;
	col.g = 0;
	col.b = 0;
	while(running)
	{
		for(x=0;x<chroma->active_device->columns_num;x++)
			for(y=0;y<chroma->active_device->rows_num;y++)
			{
				r = (cos((count+((rnd%4)*90)+y)*s)+sin(count+x)*s)*255;
				g = (cos((count+((rnd2%4)*90)+y)*s)+sin(count+x)*s)*255;
				b = (cos((count+((rnd3%4)*90)+y)*s)+sin(count+x)*s)*255;
				chroma->active_device->leds->rows[y]->column[x].r = (unsigned char)r;
				chroma->active_device->leds->rows[y]->column[x].g = (unsigned char)g;
				chroma->active_device->leds->rows[y]->column[x].b = (unsigned char)b;
				chroma->active_device->leds->update_mask |= 1<<y;
			}

		for(int i=0;i<keys_max;i++)
			if(keys_history[i]!=-1)
			{
				razer_convert_keycode_to_pos(keys_history[i],&pos);							
				razer_set_led_pos(chroma->active_device->leds,&pos,&col);
			}
		razer_update_leds(chroma,chroma->active_device->leds);
		count+=count_dir;
		if(count<=0 || count>=30)
		{
			count_dir=-count_dir;
			rnd = random();
			rnd2 = random();
			rnd3 = random();
		}
		razer_update(chroma);
		razer_frame_limiter(chroma,13);
	}
}

#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void stop(int sig)
{
	#ifdef USE_DEBUGGING
		printf("Stopping input example\n");
	#endif
	running = 0;
}

int event_handler(struct razer_chroma *chroma, struct razer_chroma_event *event)
{
	#ifdef USE_DEBUGGING
		printf("input_handler called\n");
	#endif
	if(event->type != RAZER_CHROMA_EVENT_TYPE_KEYBOARD || !event->sub_type)
		return(1);
	keys_history[keys_history_index++] = (long)event->value;
	if(keys_history_index==keys_max)
		keys_history_index = 0;
	return(1);
}

int main(int argc,char *argv[])
{
	uid_t uid = getuid();
	if(uid != 0)
		printf("input example needs root to work correctly.\n");	
	struct razer_chroma *chroma = razer_open(NULL,NULL);
	if(!chroma)
		exit(1);
 	razer_set_event_handler(chroma,event_handler);
 	razer_set_custom_mode(chroma);
	razer_clear_all(chroma->active_device->leds);
	razer_update_leds(chroma,chroma->active_device->leds);
	for(int i=0;i<10;i++)
		keys_history[i] = -1;
 	signal(SIGINT,stop);
 	signal(SIGKILL,stop);
 	signal(SIGTERM,stop);	
	effect(chroma);
 	razer_close(chroma);
}

#pragma GCC diagnostic pop

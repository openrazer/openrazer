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
		for(x=0;x<22;x++)
			for(y=0;y<6;y++)
			{
				r = (cos((count+((rnd%4)*90)+y)*s)+sin(count+x)*s)*255;
				g = (cos((count+((rnd2%4)*90)+y)*s)+sin(count+x)*s)*255;
				b = (cos((count+((rnd3%4)*90)+y)*s)+sin(count+x)*s)*255;
				chroma->keys->rows[y].column[x].r = (unsigned char)r;
				chroma->keys->rows[y].column[x].g = (unsigned char)g;
				chroma->keys->rows[y].column[x].b = (unsigned char)b;
				chroma->keys->update_mask |= 1<<y;
			}

		for(int i=0;i<keys_max;i++)
			if(keys_history[i]!=-1)
			{
				razer_convert_keycode_to_pos(keys_history[i],&pos);							
				razer_set_key_pos(chroma->keys,&pos,&col);
			}
		razer_update_keys(chroma,chroma->keys);
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

int input_handler(struct razer_chroma *chroma, int keycode,int pressed)
{
	#ifdef USE_DEBUGGING
		printf("input_handler called\n");
	#endif
	if(!pressed)
		return(1);
	keys_history[keys_history_index++] = keycode;
	if(keys_history_index==keys_max)
		keys_history_index = 0;
	return(1);
}

int main(int argc,char *argv[])
{
	uid_t uid = getuid();
	if(uid != 0)
		printf("input example needs root to work correctly.\n");	
	struct razer_chroma *chroma = razer_open();
	if(!chroma)
		exit(1);
 	razer_set_input_handler(chroma,input_handler);
 	razer_set_custom_mode(chroma);
	razer_clear_all(chroma->keys);
	razer_update_keys(chroma,chroma->keys);
	for(int i=0;i<10;i++)
		keys_history[i] = -1;
 	signal(SIGINT,stop);
 	signal(SIGKILL,stop);
 	signal(SIGTERM,stop);	
	effect(chroma);
 	razer_close(chroma);
}

#pragma GCC diagnostic pop

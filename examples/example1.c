#include "example1.h"

void effect(struct razer_chroma *chroma,struct razer_keys *keys)
{
	int r,g,b;
	int count = 0;
	int count_dir = 1;
	while(1)
	{	
		int x,y;
		for(x=0;x<22;x++)
			for(y=0;y<6;y++)
		{
			r = (count+x)*(255/22);
			g = (count-x)*(255/22);
			b = (count+y)*(255/22);

			keys->rows[y].column[x].r = (unsigned char)r;
			keys->rows[y].column[x].g = (unsigned char)g;
			keys->rows[y].column[x].b = (unsigned char)b;
			keys->update_mask |= 1<<y;
		}
		razer_update_keys(chroma,keys);
		count+=count_dir;
		if(count<=0 || count>=44)
			count_dir=-count_dir;
		usleep(60);
	}
}

int main(int argc,char *argv[])
{
	struct razer_chroma *chroma =(struct razer_chroma*)malloc(sizeof(struct razer_chroma));
 	razer_open(chroma);
    razer_set_custom_mode(chroma);
	clear_all(chroma->keys);
	razer_update_keys(chroma,chroma->keys);
	effect(chroma,chroma->keys);
 	razer_close(chroma);
 	free(chroma);
}
#include "example1.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

void effect(struct razer_chroma *chroma)
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

			chroma->keys->rows[y].column[x].r = (unsigned char)r;
			chroma->keys->rows[y].column[x].g = (unsigned char)g;
			chroma->keys->rows[y].column[x].b = (unsigned char)b;
			chroma->keys->update_mask |= 1<<y;
		}
		razer_update_keys(chroma,chroma->keys);
		count+=count_dir;
		if(count<=0 || count>=44)
			count_dir=-count_dir;
		usleep(60000);
	}
}

#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	struct razer_chroma *chroma = razer_open();
	if(!chroma)
		exit(1);
	razer_set_custom_mode(chroma);
	razer_clear_all(chroma->keys);
	razer_update_keys(chroma,chroma->keys);
	effect(chroma);
 	razer_close(chroma);
}

#pragma GCC diagnostic pop

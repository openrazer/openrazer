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
#include "example1.h"

/*usleep doesnt seem to have been defined in libc... have to investigate this someday
  ignore the warning for now
*/

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
		for(x=0;x<chroma->active_device->columns_num;x++)
			for(y=0;y<chroma->active_device->rows_num;y++)
		{
			r = (count+x)*(255/22);
			g = (count-x)*(255/22);
			b = (count+y)*(255/22);

			chroma->active_device->leds->rows[y]->column[x].r = (unsigned char)r;
			chroma->active_device->leds->rows[y]->column[x].g = (unsigned char)g;
			chroma->active_device->leds->rows[y]->column[x].b = (unsigned char)b;
			chroma->active_device->leds->update_mask |= 1<<y;
		}
		razer_update_leds(chroma,chroma->active_device->leds);
		count+=count_dir;
		if(count<=0 || count>=44)
			count_dir=-count_dir;
		usleep(60000);
	}
}

#pragma GCC diagnostic pop

/*ignore unused parameters warning*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	struct razer_chroma *chroma = razer_open(NULL,NULL);
	if(!chroma)
		exit(1);
	razer_set_custom_mode(chroma);
	razer_clear_all(chroma->active_device->leds);
	razer_update_leds(chroma,chroma->active_device->leds);
	effect(chroma);
 	razer_close(chroma);
}

#pragma GCC diagnostic pop

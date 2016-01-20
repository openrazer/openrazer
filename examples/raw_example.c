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
 #include "raw_example.h"

/*ignore unused parameters warning*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

const char *set_led_row_base_fname = "/set_key_row";
const char *set_custom_mode_base_fname = "/mode_custom";
char *device_path = NULL;

char *set_led_row_fname = NULL;
char *set_custom_mode_fname = NULL;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

int main(int argc,char *argv[])
{
	if(argc<2)
	{
		printf("no path to device driver attribute files given.Exiting...\n");
		exit(1);
	}
	device_path = argv[1];
	printf("preparing filenames for device path:%s\n",device_path);
	//opening device driver attribute files
	set_custom_mode_fname = (char*)malloc(strlen(device_path)+strlen(set_custom_mode_base_fname)+1);
	set_led_row_fname = (char*)malloc(strlen(device_path)+strlen(set_led_row_base_fname)+1);
	memset((void*)set_custom_mode_fname,0,strlen(device_path)+strlen(set_custom_mode_base_fname)+1);
	memcpy(set_custom_mode_fname,device_path,strlen(device_path));
	memcpy(set_custom_mode_fname+strlen(device_path),set_custom_mode_base_fname,strlen(set_custom_mode_base_fname));
	memset((void*)set_led_row_fname,0,strlen(device_path)+strlen(set_led_row_base_fname)+1);
	memcpy(set_led_row_fname,device_path,strlen(device_path));
	memcpy(set_led_row_fname+strlen(device_path),set_led_row_base_fname,strlen(set_led_row_base_fname));
	FILE *set_led_row = fopen(set_led_row_fname,"w");
	FILE *set_custom_mode = fopen(set_custom_mode_fname,"w");
	printf("opening device driver files: [%s,%s]\n",set_custom_mode_fname,set_led_row_fname);
	if(!set_led_row || !set_custom_mode)
	{
		printf("error opening device driver attribute files.Exiting...\n");
		exit(1);
	}
	unsigned char *cols = (unsigned char*)malloc(15*3+1);//this will hold the colors for the firefly
	cols[0] = 0; //the firefly only uses one row,index will always be zero 
	int count = 2000;
	srand(time(NULL));
	while(count--)
	{
	printf("round:%d\n",2000-count);
	for(int i =1;i<((15*3)+1);i++)
	{
		if(count>1000)
		cols[i] = (unsigned char)(rand()*255); //randomize colors each round
		else cols[i] = count&0xff;
	}
	fwrite(cols,15*3+1,1,set_led_row);
	fflush(set_led_row);//flush buffers

	fwrite("1",1,1,set_custom_mode);//update leds
	fflush(set_custom_mode);
	usleep(6000);
	}
	fclose(set_led_row);//close attribute files
	fclose(set_custom_mode);
	printf("Raw example finished.\n");
}

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

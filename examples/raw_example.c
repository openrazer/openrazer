#include "raw_example.h"

/*ignore unused parameters warning*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

const char *set_key_row_base_fname = "/set_key_row";
const char *set_custom_mode_base_fname = "/mode_custom";
char *device_path = NULL;

char *set_key_row_fname = NULL;
char *set_custom_mode_fname = NULL;


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
	set_key_row_fname = (char*)malloc(strlen(device_path)+strlen(set_key_row_base_fname)+1);
	memset((void*)set_custom_mode_fname,0,strlen(device_path)+strlen(set_custom_mode_base_fname)+1);
	memcpy(set_custom_mode_fname,device_path,strlen(device_path));
	memcpy(set_custom_mode_fname+strlen(device_path),set_custom_mode_base_fname,strlen(set_custom_mode_base_fname));
	memset((void*)set_key_row_fname,0,strlen(device_path)+strlen(set_key_row_base_fname)+1);
	memcpy(set_key_row_fname,device_path,strlen(device_path));
	memcpy(set_key_row_fname+strlen(device_path),set_key_row_base_fname,strlen(set_key_row_base_fname));
	FILE *set_key_row = fopen(set_key_row_fname,"w");
	FILE *set_custom_mode = fopen(set_custom_mode_fname,"w");
	printf("opening device driver files: [%s,%s]\n",set_custom_mode_fname,set_key_row_fname);
	if(!set_key_row || !set_custom_mode)
	{
		printf("error opening device driver attribute files.Exiting...\n");
		exit(1);
	}
	unsigned char *cols = (unsigned char*)malloc(15*3+1);
	cols[0] = 0; //the firefly only uses one row,index will always be zero 
	int count = 2000;
	srand(time(NULL));
	while(count--)
	{
	printf("round:%d\n",2000-count);
	for(int i =1;i<((15*3)+1);i++)
	{
		cols[i] = (unsigned char)(rand()*255);
		//cols[i] = 0;
	}
	fwrite(cols,15*3+1,1,set_key_row);
	fflush(set_key_row);

	fwrite("1",1,1,set_custom_mode);
	fflush(set_custom_mode);

	usleep(6000);
	}
	fclose(set_key_row);
	fclose(set_custom_mode);
}

#pragma GCC diagnostic pop

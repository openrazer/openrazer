#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "../lib/razer_chroma.h"

/*ignore unused parameters warning*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
  
  
  struct razer_chroma *klaw=razer_open();
  if(!klaw) {
    printf("failed to initialize!!!\n");
    exit(0);
  }
  razer_set_custom_mode(klaw);
  razer_clear_all(klaw->keys);
  razer_update_keys(klaw, klaw->keys);
  
  struct razer_rgb color;
  struct razer_pos pos;

  while(1){
    int cmd;
    scanf("%u", &cmd);
    if(cmd == 1){
      int x = 0, y = 0;
      int ir, ig, ib;
    
      printf("Enter x y r g b: ");
      scanf("%u %u %u %u %u", &x, &y, &ir, &ig, &ib);
      printf("Setting key at [%u,%u] to #%x%x%x...", x, y, ir, ig, ib);
      color.r = ir;
      color.g = ig;
      color.b = ib;
      pos.x = x;
      pos.y = y;
      razer_set_key_pos(klaw->keys, &pos, &color);
      razer_update_keys(klaw, klaw->keys);
      printf("Done!\n");
    }
    if(cmd == 2){
      char targs[] = ""; //dziala, char targs[255]
      int ir, ig, ib;
    
      printf("Enter r g b: ");
      scanf("%u %u %u", &ir, &ig, &ib);
      printf("Enter str: ");
      scanf("%s", targs);
      printf("Setting keys of [%s] to %u %u %u...\n", targs, ir, ig, ib);
      color.r = ir;
      color.g = ig;
      color.b = ib;
      
      for(unsigned int i = 0; i < strlen(targs); i++){
	printf("Setting %c...\n", targs[i]);
	razer_convert_ascii_to_pos(targs[i],&pos);
	razer_set_key_pos(klaw->keys,&pos,&color);
      }
      razer_update_keys(klaw, klaw->keys);
      printf("Done!\n");
   
    }
    
    if(cmd == 3) {
      int ir, ig, ib; 
      printf("Enter r g b: ");
      scanf("%u %u %u", &ir, &ig, &ib);
      color.r = ir; 
      color.g = ig; 
      color.b = ib; 
      printf("Setting keys to %u %u %u...\n", ir, ig, ib);
      razer_set_all(klaw->keys, &color);
      razer_update_keys(klaw, klaw->keys);
      printf("Done!\n");
    }


    if(cmd==0) break;
    
    }
  printf("Exiting...");
  razer_close(klaw);
}

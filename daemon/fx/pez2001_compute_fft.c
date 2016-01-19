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
 #include "pez2001_compute_fft.h"


const char RIFF_MAGIC[] = "RIFF";
const char WAVE_MAGIC[] = "WAVE";
const char WAVE_CHUNK_MAGIC[] = "fmt ";
const char WAVE_CHUNK_DATA_MAGIC[] = "data";

void close_wav(wav_file *wf)
{
 if(wf==NULL)
  return;
 fclose(wf->file);
 free(wf->filename);
 free(wf->header);
 if(wf->sample_buffer!=NULL)
  free(wf->sample_buffer);
 free(wf);
}

unsigned long get_samplerate(wav_file *wf)
{
 return(wf->header->samplerate);
}

unsigned short get_channels(wav_file *wf)
{
 return(wf->header->channels);
}

unsigned long get_bitrate(wav_file *wf)
{
 return(wf->header->bitrate);
}

unsigned long get_bits_per_sample(wav_file *wf)
{
 return(wf->header->bits_per_sample);
}

unsigned short get_format_tag(wav_file *wf)
{
 return(wf->header->format_tag);
}

unsigned short get_block_align(wav_file *wf)
{
 return(wf->header->block_align);
}

unsigned long get_actual_sample_index(wav_file *wf)
{
 return( (wf->offset-WAVE_HEADER_LENGTH + wf->sample_buffer_offset )/wf->header->block_align);
}

int fill_sample_buffer(wav_file *wf)
{
 int samples_read = fread((void*)wf->sample_buffer,1,wf->sample_buffer_length,wf->file);
 wf->offset+=samples_read;
 wf->sample_buffer_used = samples_read;
 wf->sample_buffer_offset = 0;
 return(samples_read);
}

wav_file *open_wav(char *filename)
{
 wav_file *state = (wav_file*)malloc(sizeof(wav_file));
 state->file = fopen(filename,"rb");
 if(state->file==NULL)
  {
   free(state);
   return(NULL);
  }
 state->filename = (char*)malloc(strlen(filename)+1);
 strcpy(state->filename,filename);
 state->header = (wav_header*)malloc(sizeof(wav_header));
 state->sample_buffer=NULL;
 state->sample_buffer_length=0;
 memset(state->header,0,sizeof(wav_header));
 int r = fread((void*)state->header,sizeof(wav_header),1,state->file);
 if(!r)
 {
  close_wav(state);
  return(NULL);
 }
 /*header sanity check*/
 if(memcmp(state->header->riff_magic,&RIFF_MAGIC,4) || memcmp(state->header->wave_magic,&WAVE_MAGIC,4)
 || memcmp(state->header->wave_chunk_magic,&WAVE_CHUNK_MAGIC,4) || memcmp(state->header->wave_chunk_data_magic,&WAVE_CHUNK_DATA_MAGIC,4) || state->header->format_tag != PCM_FORMAT_TAG)
 {
  printf("sanity check failed\n");
  close_wav(state);
  return(NULL);
 }
 state->offset = WAVE_HEADER_LENGTH; //set offset to beginning of data
 printf("samplerate: %dhz\n",get_samplerate(state));
 printf("bitrate: %d\n",state->header->bitrate);
 printf("channels: %d\n",state->header->channels);
 printf("bits per sample: %d\n",state->header->bits_per_sample);
 printf("block align: %d\n",state->header->block_align);
 printf("format_tag: %d\n",state->header->format_tag);
 state->samples_num = state->header->wave_chunk_data_length/state->header->block_align;
 //use a 1000 samples buffer
 unsigned long sample_buffer_num = 1000;
 if(state->samples_num<sample_buffer_num)
  sample_buffer_num = state->samples_num;
 state->sample_buffer_length = state->header->block_align*sample_buffer_num;
 state->sample_buffer = (void*)malloc(state->sample_buffer_length);
 printf("created a %d bytes samples buffer\n",state->sample_buffer_length);
 printf("number of samples: %d\n",state->samples_num);
 if(!fill_sample_buffer(state))
 {
  close_wav(state);
  return(NULL);
 }
 return(state);
}

short read_wav_sample(wav_file *wf)
{
 short left = (((unsigned char*)wf->sample_buffer)[wf->sample_buffer_offset+1]<<8) + ((unsigned char*)wf->sample_buffer)[wf->sample_buffer_offset];
 wf->sample_buffer_offset+=get_block_align(wf);
 /*automatic buffer filling - may increase read time -> use polling mechanism for more stable function timings*/
 if(wf->sample_buffer_offset>=wf->sample_buffer_length)
  fill_sample_buffer(wf);
 return(left);
}

unsigned int read_wav_stereo_sample(wav_file *wf)
{
 unsigned int both = ((unsigned int*)wf->sample_buffer)[wf->sample_buffer_offset/4];
 wf->sample_buffer_offset+=get_block_align(wf);
 /*automatic buffer filling - may increase read time -> use polling mechanism for more stable function timings*/
 if(wf->sample_buffer_offset>=wf->sample_buffer_length)
  fill_sample_buffer(wf);
 return(both);
}

int wav_samples_left(wav_file *wf)
{
 return(((WAVE_HEADER_LENGTH + wf->header->wave_chunk_data_length)-wf->offset)-wf->sample_buffer_offset);
}



double *create_hamming_window_buffer(int size)
{
  double *buffer = (double*)malloc(sizeof(double)*size);
  int i;
  for(i=0;i<size;i++)
  {
    buffer[i] = 0.54-(0.46*cos(2*M_PI*(i/((size-1)*1.0))));
  }
  return(buffer);
}



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"


struct razer_effect *effect = NULL;
wav_file *effect_input_file = NULL;

fftw_complex  *effect_fft_in,*effect_fft_out;
fftw_plan effect_fft_plan;
double *effect_fft_hamming_buffer = NULL;
unsigned long effect_fft_samples = 512;
unsigned long effect_fft_samples_used = 0;


int effect_update(struct razer_fx_render_node *render)
{
	float magnitude = daemon_get_parameter_float(daemon_effect_get_parameter_by_index(render->effect,1));
	int x,y;
	struct razer_rgb col;
	#ifdef USE_DEBUGGING
		printf(" (Fft.%d ## %%:%f)",render->id);
	#endif

	unsigned long samples_left=0;
	while((samples_left=wav_samples_left(effect_input_file)))
	{
		unsigned int sample = read_wav_stereo_sample(effect_input_file);
		short high = sample >> 16;
		//short low = sample & 0xFFFF;
		//add sample to fft buffer
		effect_fft_in[effect_fft_samples_used][0] = (double)high * effect_fft_hamming_buffer[effect_fft_samples_used];///(double)32768;//* windowHanning(step++, N);
  		effect_fft_in[effect_fft_samples_used++][1] = 0.0f;
		//enough samples gathered?
		if(effect_fft_samples_used==effect_fft_samples)
  		{
  			printf("Computing fft, still %d samples left\n",samples_left);
			//compute fft
    		effect_fft_plan = fftw_plan_dft_1d(effect_fft_samples, effect_fft_in, effect_fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    		fftw_execute(effect_fft_plan);
    		fftw_destroy_plan(effect_fft_plan);
    		effect_fft_samples_used = 0;
 		    double tmp_magnitude = sqrt(effect_fft_out[0][0]*effect_fft_out[0][0] + effect_fft_out[0][1]*effect_fft_out[0][1]);
		    tmp_magnitude = 10./log(10.) * log(tmp_magnitude + 1e-6);
    		printf("new fft mag db:%f\n",tmp_magnitude);
    		double sum = 0.0f;
    		for(unsigned int i=0;i<effect_fft_samples/2;i++)
    		{
	 		    double tmp_bin_magnitude = sqrt(effect_fft_out[i][0]*effect_fft_out[i][0] + effect_fft_out[i][1]*effect_fft_out[i][1]);
			    tmp_bin_magnitude = 10./log(10.) * log(tmp_bin_magnitude + 1e-6);
			    sum += tmp_bin_magnitude;
    		}
    		printf("sum:%f\n",sum/(effect_fft_samples/2));
    		magnitude = (float)tmp_magnitude - 50.0f;
    		break;
  		}
	}
	if(!samples_left)
	{
		#ifdef USE_DEBUGGING
			printf("no samples left to analyze, closing input file\n");
		#endif
		close_wav(effect_input_file);
		effect_input_file = NULL;
		return(0);
	}


	//set color to avg magnitude ,transformed to 0.0-1.0 space	

	//calculate hue from magnitude
	rgb_from_hue(magnitude/96,0.3f,0.0f,&col);

	for(x=0;x<render->device->columns_num;x++)
		for(y=0;y<render->device->rows_num;y++)
		{
			rgb_mix_into(&render->output_frame->rows[y]->column[x],&render->input_frame->rows[y]->column[x],&col,render->opacity);//*render->opacity  //&render->second_input_frame->rows[y]->column[x]
			render->output_frame->update_mask |= 1<<y;
		}
	daemon_set_parameter_float(daemon_effect_get_parameter_by_index(render->effect,1),magnitude);	
	return(1);
}

int effect_reset(struct razer_fx_render_node *render)
{
	//reopen input file
	char *filename = daemon_get_parameter_string(daemon_effect_get_parameter_by_index(render->effect,0));
	if(effect_input_file)
	{
		close_wav(effect_input_file);
		effect_input_file = NULL;
	}
	effect_input_file =open_wav(filename);
	#ifdef USE_DEBUGGING
		printf("(fft) opened input wav file:%s,%x\n",filename);
	#endif		
	if(effect_input_file)
		return(1);
	else
		return(0);
}

#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

void fx_init(struct razer_daemon *daemon)
{
	srand(time(NULL));

	effect_fft_in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*effect_fft_samples);
	effect_fft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*effect_fft_samples);
 	effect_fft_hamming_buffer = create_hamming_window_buffer(effect_fft_samples);

	struct razer_parameter *parameter = NULL;
	effect = daemon_create_effect();
	effect->update = effect_update;
	effect->reset = effect_reset;
	effect->name = "Simple Spectrum display #1";
	effect->description = "Spectrum analyzer for sound input devices";
	effect->fps = 15;
	effect->effect_class = 1;
	effect->input_usage_mask = RAZER_EFFECT_FIRST_INPUT_USED;
	parameter = daemon_create_parameter_string("Effect Input Device","Filepath pointing to the sound input device/file(wav format) (STRING)","/dev/dsp");
	daemon_effect_add_parameter(effect,parameter);	
	parameter = daemon_create_parameter_float("Effect Magnitude","actual spectrum energy display(FLOAT)",0.0f);
	daemon_effect_add_parameter(effect,parameter);	
	int effect_uid = daemon_register_effect(daemon,effect);
	#ifdef USE_DEBUGGING
		printf("registered effect: %s (uid:%d)\n",effect->name,effect->id);
	#endif
	


}

#pragma GCC diagnostic pop


void fx_shutdown(struct razer_daemon *daemon)
{
	daemon_unregister_effect(daemon,effect); //TODO do this automatically when the daemon closes - so an effects prodrammer will use this exported function only on special occasions
	daemon_free_parameters(effect->parameters);
	daemon_free_effect(effect);

	fftw_free(effect_fft_in);
 	fftw_free(effect_fft_out);

	//close input file
}

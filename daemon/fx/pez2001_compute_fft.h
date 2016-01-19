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
 #ifndef _PEZ2001_COMPUTE_FFT_H_
#define _PEZ2001_COMPUTE_FFT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <fftw3.h>

#include "../razer_daemon.h"


#define PCM_FORMAT_TAG 0x01
#define WAVE_HEADER_LENGTH 0x28

//#pragma pack(push,1)
//#pragma pack(push)  /* push current alignment to stack */
//#pragma pack(1)     /* set alignment to 1 byte boundary */
//__attribute__((packed))
struct wav_header_struct
{
 /*riff format header*/
 char riff_magic[4];
 unsigned int length;
 char wave_magic[4];

 /*wave chunk header part*/
 char wave_chunk_magic[4];
 unsigned int wave_chunk_length;
 unsigned short format_tag;
 unsigned short channels;
 unsigned int samplerate;
 unsigned int bitrate;
 unsigned short block_align;
 unsigned short bits_per_sample;

 /*wave chunk data header*/
 char wave_chunk_data_magic[4];
 unsigned int wave_chunk_data_length;

} __attribute__((__packed__));

typedef struct wav_header_struct wav_header;
//#pragma pack(pop)

typedef struct
{
 FILE *file;
 char *filename;
 long offset;
 wav_header *header;
 unsigned long samples_num;
 void *sample_buffer;
 unsigned long sample_buffer_length;
 unsigned long sample_buffer_offset;
 unsigned long sample_buffer_used;
} wav_file;

void close_wav(wav_file *wf);

unsigned long get_samplerate(wav_file *wf);

unsigned short get_channels(wav_file *wf);

unsigned long get_bitrate(wav_file *wf);

unsigned long get_bits_per_sample(wav_file *wf);

unsigned short get_format_tag(wav_file *wf);

unsigned short get_block_align(wav_file *wf);

unsigned long get_actual_sample_index(wav_file *wf);

int fill_sample_buffer(wav_file *wf);

wav_file *open_wav(char *filename);

short read_wav_sample(wav_file *wf);

int wav_samples_left(wav_file *wf);




#endif

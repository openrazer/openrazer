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
#ifndef _RAZER_DAEMON_TYPES_H_
#define _RAZER_DAEMON_TYPES_H_

typedef unsigned long razer_uint;
typedef long razer_int;
typedef float razer_float;
typedef int razer_index;

struct razer_float_range 
{
	float min;
	float max;
};

struct razer_int_range
{
	long min;
	long max;
};

struct razer_uint_range
{
	unsigned long min;
	unsigned long max;
};

struct razer_rgb_range
{
	struct razer_rgb *min;
	struct razer_rgb *max;
};

struct razer_pos_range
{
	struct razer_pos *min;
	struct razer_pos *max;
};

struct razer_array_header
{
	int size;
	int has_fixed_size;
};

struct razer_float_array //list *array //struct parameters
{
	int size;
	int has_fixed_size;
	float *values;
};
struct razer_int_array
{
	int size;
	int has_fixed_size;
	long *values;
};

struct razer_uint_array
{
	int size;
	int has_fixed_size;
	unsigned long *values;
};

struct razer_rgb_array
{
	int size;
	int has_fixed_size;
	struct razer_rgb **values;
};

struct razer_pos_array
{
	int size;
	int has_fixed_size;
	struct razer_pos **values;
};


struct razer_float_array *daemon_create_float_array(int size,int has_fixed_size);
struct razer_int_array *daemon_create_int_array(int size,int has_fixed_size);
struct razer_uint_array *daemon_create_uint_array(int size,int has_fixed_size);
struct razer_rgb_array *daemon_create_rgb_array(int size,int has_fixed_size);
struct razer_pos_array *daemon_create_pos_array(int size,int has_fixed_size);

struct razer_float_range *daemon_create_float_range(float min,float max);
struct razer_int_range *daemon_create_int_range(long min,long max);
struct razer_uint_range *daemon_create_uint_range(unsigned long min,unsigned long max);
struct razer_rgb_range *daemon_create_rgb_range(struct razer_rgb *min ,struct razer_rgb *max);
struct razer_pos_range *daemon_create_pos_range(struct razer_pos *min ,struct razer_pos *max);



struct razer_float_range *razer_float_range_copy(struct razer_float_range *range);
struct razer_int_range *razer_int_range_copy(struct razer_int_range *range);
struct razer_uint_range *razer_uint_range_copy(struct razer_uint_range *range);
struct razer_rgb_range *razer_rgb_range_copy(struct razer_rgb_range *range);
struct razer_pos_range *razer_pos_range_copy(struct razer_pos_range *range);
struct razer_float_array *razer_float_array_copy(struct razer_float_array *array);
struct razer_int_array *razer_int_array_copy(struct razer_int_array *array);
struct razer_uint_array *razer_uint_array_copy(struct razer_uint_array *array);
struct razer_rgb_array *razer_rgb_array_copy(struct razer_rgb_array *array);
struct razer_pos_array *razer_pos_array_copy(struct razer_pos_array *array);

#endif
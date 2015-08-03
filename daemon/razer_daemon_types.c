#include "razer_daemon.h"

struct razer_float_array *daemon_create_float_array(int size,int has_fixed_size)
{
	struct razer_float_array *array = (struct razer_float_array*)malloc(sizeof(struct razer_float_array));
	array->values = (float*)malloc(sizeof(float*)*size);
	memset(array->values,0,sizeof(float*)*size);
	array->size = size;
	array->has_fixed_size = has_fixed_size;
	return(array);
}

struct razer_int_array *daemon_create_int_array(int size,int has_fixed_size)
{
	struct razer_int_array *array = (struct razer_int_array*)malloc(sizeof(struct razer_int_array));
	array->values = (long*)malloc(sizeof(long*)*size);
	memset(array->values,0,sizeof(float*)*size);
	array->size = size;
	array->has_fixed_size = has_fixed_size;
	return(array);
}

struct razer_uint_array *daemon_create_uint_array(int size,int has_fixed_size)
{
	struct razer_uint_array *array = (struct razer_uint_array*)malloc(sizeof(struct razer_uint_array));
	array->values = (unsigned long*)malloc(sizeof(unsigned long*)*size);
	memset(array->values,0,sizeof(float*)*size);
	array->size = size;
	array->has_fixed_size = has_fixed_size;
	return(array);
}

struct razer_rgb_array *daemon_create_rgb_array(int size,int has_fixed_size)
{
	struct razer_rgb_array *array = (struct razer_rgb_array*)malloc(sizeof(struct razer_rgb_array));
	array->values = (struct razer_rgb*)malloc(sizeof(struct razer_rgb*)*size);
	memset(array->values,0,sizeof(float*)*size);
	array->size = size;
	array->has_fixed_size = has_fixed_size;
	return(array);
}

struct razer_pos_array *daemon_create_pos_array(int size,int has_fixed_size)
{
	struct razer_pos_array *array = (struct razer_pos_array*)malloc(sizeof(struct razer_pos_array));
	array->values = (struct razer_pos*)malloc(sizeof(struct razer_pos*)*size);
	memset(array->values,0,sizeof(float*)*size);
	array->size = size;
	array->has_fixed_size = has_fixed_size;
	return(array);
}

struct razer_float_range *daemon_create_float_range(float min,float max)
{
	struct razer_float_range *range = (struct razer_float_range*)malloc(sizeof(struct razer_float_range));
	range->min = min;
	range->max = max;
	return(range);
}

struct razer_int_range *daemon_create_int_range(long min,long max)
{
	struct razer_int_range *range = (struct razer_int_range*)malloc(sizeof(struct razer_int_range));
	range->min = min;
	range->max = max;
	return(range);
}

struct razer_uint_range *daemon_create_uint_range(unsigned long min,unsigned long max)
{
	struct razer_uint_range *range = (struct razer_uint_range*)malloc(sizeof(struct razer_uint_range));
	range->min = min;
	range->max = max;
	return(range);
}
struct razer_rgb_range *daemon_create_rgb_range(struct razer_rgb *min ,struct razer_rgb *max)
{
	struct razer_rgb_range *range = (struct razer_rgb_range*)malloc(sizeof(struct razer_rgb_range));
	range->min = min;
	range->max = max;
	return(range);
}

struct razer_pos_range *daemon_create_pos_range(struct razer_pos *min ,struct razer_pos *max)
{
	struct razer_pos_range *range = (struct razer_pos_range*)malloc(sizeof(struct razer_pos_range));
	range->min = min;
	range->max = max;
	return(range);
}




struct razer_float_range *razer_float_range_copy(struct razer_float_range *range)
{
	struct razer_float_range *copy = (struct razer_float_range*)malloc(sizeof(struct razer_float_range));
	copy->min = range->min;
	copy->max = range->max;
	return(copy);
}

struct razer_int_range *razer_int_range_copy(struct razer_int_range *range)
{
	struct razer_int_range *copy = (struct razer_int_range*)malloc(sizeof(struct razer_int_range));
	copy->min = range->min;
	copy->max = range->max;
	return(copy);
}

struct razer_uint_range *razer_uint_range_copy(struct razer_uint_range *range)
{
	struct razer_uint_range *copy = (struct razer_uint_range*)malloc(sizeof(struct razer_uint_range));
	copy->min = range->min;
	copy->max = range->max;
	return(copy);
}

struct razer_rgb_range *razer_rgb_range_copy(struct razer_rgb_range *range)
{
	struct razer_rgb_range *copy = (struct razer_rgb_range*)malloc(sizeof(struct razer_rgb_range));
	copy->min = rgb_copy(range->min);
	copy->max = rgb_copy(range->max);
	return(copy);
}

struct razer_pos_range *razer_pos_range_copy(struct razer_pos_range *range)
{
	struct razer_pos_range *copy = (struct razer_pos_range*)malloc(sizeof(struct razer_pos_range));
	copy->min = razer_pos_copy(range->min);
	copy->max = razer_pos_copy(range->max);
	return(copy);
}

struct razer_float_array *razer_float_array_copy(struct razer_float_array *array)
{
	struct razer_float_array *copy = (struct razer_float_array*)malloc(sizeof(struct razer_float_array));
	copy->values = (float*)malloc(sizeof(float)*array->size);
	memcpy(copy->values,array->values,sizeof(float)*array->size);
	copy->size = array->size;
	copy->has_fixed_size = array->has_fixed_size;
	return(copy);
}

struct razer_int_array *razer_int_array_copy(struct razer_int_array *array)
{
	struct razer_int_array *copy = (struct razer_int_array*)malloc(sizeof(struct razer_int_array));
	copy->values = (long*)malloc(sizeof(long)*array->size);
	memcpy(copy->values,array->values,sizeof(long)*array->size);
	copy->size = array->size;
	copy->has_fixed_size = array->has_fixed_size;
	return(copy);
}

struct razer_uint_array *razer_uint_array_copy(struct razer_uint_array *array)
{
	struct razer_uint_array *copy = (struct razer_uint_array*)malloc(sizeof(struct razer_uint_array));
	copy->values = (unsigned long*)malloc(sizeof(unsigned long)*array->size);
	memcpy(copy->values,array->values,sizeof(unsigned long)*array->size);
	copy->size = array->size;
	copy->has_fixed_size = array->has_fixed_size;
	return(copy);
}

struct razer_rgb_array *razer_rgb_array_copy(struct razer_rgb_array *array)
{
	struct razer_rgb_array *copy = (struct razer_rgb_array*)malloc(sizeof(struct razer_rgb_array));
	copy->values = (struct razer_rgb*)malloc(sizeof(struct razer_rgb*)*array->size);
	memcpy(copy->values,array->values,sizeof(struct razer_rgb*)*array->size);
	copy->size = array->size;
	copy->has_fixed_size = array->has_fixed_size;
	return(copy);
}

struct razer_pos_array *razer_pos_array_copy(struct razer_pos_array *array)
{
	struct razer_pos_array *copy = (struct razer_pos_array*)malloc(sizeof(struct razer_pos_array));
	copy->values = (struct razer_pos*)malloc(sizeof(struct razer_pos*)*array->size);
	memcpy(copy->values,array->values,sizeof(struct razer_pos*)*array->size);
	copy->size = array->size;
	copy->has_fixed_size = array->has_fixed_size;
	return(copy);
}

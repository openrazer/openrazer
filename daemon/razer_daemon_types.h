#ifndef _RAZER_DAEMON_TYPES_H_
#define _RAZER_DAEMON_TYPES_H_

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

struct razer_float_array //list *array //struct parameters
{
	float *values;
	int size;
	int has_fixed_size;
};
struct razer_int_array
{
	long *values;
	int size;
	int has_fixed_size;
};

struct razer_uint_array
{
	unsigned long *values;
	int size;
	int has_fixed_size;
};

struct razer_rgb_array
{
	struct razer_rgb **values;
	int size;
	int has_fixed_size;
};

struct razer_pos_array
{
	struct razer_pos **values;
	int size;
	int has_fixed_size;
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
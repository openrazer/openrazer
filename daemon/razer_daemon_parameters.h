#ifndef _RAZER_DAEMON_PARAMETERS_H_
#define _RAZER_DAEMON_PARAMETERS_H_


#define RAZER_PARAMETER_TYPE_UNKNOWN 0
#define RAZER_PARAMETER_TYPE_STRING 1
#define RAZER_PARAMETER_TYPE_INT 2
#define RAZER_PARAMETER_TYPE_FLOAT 3
#define RAZER_PARAMETER_TYPE_RGB 4
#define RAZER_PARAMETER_TYPE_RENDER_NODE 5
#define RAZER_PARAMETER_TYPE_UINT 6
#define RAZER_PARAMETER_TYPE_FLOAT_RANGE 7
#define RAZER_PARAMETER_TYPE_INT_RANGE 8
#define RAZER_PARAMETER_TYPE_UINT_RANGE 9
#define RAZER_PARAMETER_TYPE_RGB_RANGE 10
#define RAZER_PARAMETER_TYPE_INT_ARRAY 11
#define RAZER_PARAMETER_TYPE_UINT_ARRAY 12
#define RAZER_PARAMETER_TYPE_FLOAT_ARRAY 13
#define RAZER_PARAMETER_TYPE_RGB_ARRAY 14
#define RAZER_PARAMETER_TYPE_POS_ARRAY 15
#define RAZER_PARAMETER_TYPE_POS_RANGE 16
#define RAZER_PARAMETER_TYPE_POS 17

//#define RAZER_PARAMETER_TYPE_F 5
//#define RAZER_PARAMETER_TYPE_MATH_OP 5
//TODO RANDOM TYPES (Range included)


struct razer_parameter
{
	int id;
	char *key;
	char *description;
	unsigned long value;
	int type;
	int private;
};

char *daemon_parameter_to_json(struct razer_parameter *parameter, int final);
char *daemon_parameter_array_to_json(struct razer_parameter *parameter,int array_index);

char *daemon_parameter_type_to_string(struct razer_parameter *parameter);



struct razer_parameter *daemon_create_parameter_string(char *key,char *description,char *value);
struct razer_parameter *daemon_create_parameter_float(char *key,char *description,float value);
struct razer_parameter *daemon_create_parameter_int(char *key,char *description,long value);
struct razer_parameter *daemon_create_parameter_uint(char *key,char *description,unsigned long value);
struct razer_parameter *daemon_create_parameter_rgb(char *key,char *description,struct razer_rgb *value);
struct razer_parameter *daemon_create_parameter_pos(char *key,char *description,struct razer_pos *value);
struct razer_parameter *daemon_create_parameter_render_node(char *key,char *description,struct razer_fx_render_node *value);
struct razer_parameter *daemon_create_parameter_float_range(char *key,char *description,struct razer_float_range *value);
struct razer_parameter *daemon_create_parameter_int_range(char *key,char *description,struct razer_int_range *value);
struct razer_parameter *daemon_create_parameter_uint_range(char *key,char *description,struct razer_uint_range *value);
struct razer_parameter *daemon_create_parameter_rgb_range(char *key,char *description,struct razer_rgb_range *value);
struct razer_parameter *daemon_create_parameter_pos_range(char *key,char *description,struct razer_pos_range *value);
struct razer_parameter *daemon_create_parameter_float_array(char *key,char *description,struct razer_float_array *value);
struct razer_parameter *daemon_create_parameter_int_array(char *key,char *description,struct razer_int_array *value);
struct razer_parameter *daemon_create_parameter_uint_array(char *key,char *description,struct razer_uint_array *value);
struct razer_parameter *daemon_create_parameter_rgb_array(char *key,char *description,struct razer_rgb_array *value);
struct razer_parameter *daemon_create_parameter_pos_array(char *key,char *description,struct razer_pos_array *value);


void daemon_set_parameter_string(struct razer_parameter *parameter,char *value);
void daemon_set_parameter_float(struct razer_parameter *parameter,float value);
void daemon_set_parameter_int(struct razer_parameter *parameter,long value);
void daemon_set_parameter_uint(struct razer_parameter *parameter,unsigned long value);
void daemon_set_parameter_rgb(struct razer_parameter *parameter,struct razer_rgb *value);
void daemon_set_parameter_pos(struct razer_parameter *parameter,struct razer_pos *value);
void daemon_set_parameter_render_node(struct razer_parameter *parameter,struct razer_fx_render_node *value);
void daemon_set_parameter_float_range(struct razer_parameter *parameter,struct razer_float_range *value);
void daemon_set_parameter_int_range(struct razer_parameter *parameter,struct razer_int_range *value);
void daemon_set_parameter_uint_range(struct razer_parameter *parameter,struct razer_uint_range *value);
void daemon_set_parameter_rgb_range(struct razer_parameter *parameter,struct razer_rgb_range *value);
void daemon_set_parameter_pos_range(struct razer_parameter *parameter,struct razer_pos_range *value);
void daemon_set_parameter_float_array(struct razer_parameter *parameter,struct razer_float_array *value);
void daemon_set_parameter_int_array(struct razer_parameter *parameter,struct razer_int_array *value);
void daemon_set_parameter_uint_array(struct razer_parameter *parameter,struct razer_uint_array *value);
void daemon_set_parameter_rgb_array(struct razer_parameter *parameter,struct razer_rgb_array *value);
void daemon_set_parameter_pos_array(struct razer_parameter *parameter,struct razer_pos_array *value);


char *daemon_get_parameter_string(struct razer_parameter *parameter);
float daemon_get_parameter_float(struct razer_parameter *parameter);
long daemon_get_parameter_int(struct razer_parameter *parameter);
unsigned long daemon_get_parameter_uint(struct razer_parameter *parameter);
struct razer_rgb *daemon_get_parameter_rgb(struct razer_parameter *parameter);
struct razer_pos *daemon_get_parameter_pos(struct razer_parameter *parameter);
struct razer_fx_render_node *daemon_get_parameter_render_node(struct razer_parameter *parameter);
struct razer_float_range *daemon_get_parameter_float_range(struct razer_parameter *parameter);
struct razer_int_range *daemon_get_parameter_int_range(struct razer_parameter *parameter);
struct razer_uint_range *daemon_get_parameter_uint_range(struct razer_parameter *parameter);
struct razer_rgb_range *daemon_get_parameter_rgb_range(struct razer_parameter *parameter);
struct razer_pos_range *daemon_get_parameter_pos_range(struct razer_parameter *parameter);
struct razer_float_array *daemon_get_parameter_float_array(struct razer_parameter *parameter);
struct razer_int_array *daemon_get_parameter_int_array(struct razer_parameter *parameter);
struct razer_uint_array *daemon_get_parameter_uint_array(struct razer_parameter *parameter);
struct razer_rgb_array *daemon_get_parameter_rgb_array(struct razer_parameter *parameter);
struct razer_pos_array *daemon_get_parameter_pos_array(struct razer_parameter *parameter);


struct razer_parameter *daemon_copy_parameter(struct razer_parameter *parameter);
void daemon_free_parameter(struct razer_parameter *parameter);
void daemon_free_parameters(list *parameters);


struct razer_effect;


struct razer_parameter *daemon_remove_parameter(list *parameters,char *key,int type);
struct razer_parameter *daemon_get_parameter(list *parameters,char *key,int type);




#endif

#include "razer_daemon.h"

char *daemon_parameter_to_json(struct razer_parameter *parameter)
{
	char *parameter_json = str_CreateEmpty();
	parameter_json = str_CatFree(parameter_json,"{\n \"key\": \"");
	parameter_json = str_CatFree(parameter_json,parameter->key);
	parameter_json = str_CatFree(parameter_json,"\",\n");
	parameter_json = str_CatFree(parameter_json," \"id\" : ");
	char *id_string = str_FromLong(parameter->id);
	parameter_json = str_CatFree(parameter_json,id_string);
	parameter_json = str_CatFree(parameter_json," ,\n");
	free(id_string);
	parameter_json = str_CatFree(parameter_json," \"type\" : ");
	char *type_string = str_FromLong(parameter->type);
	parameter_json = str_CatFree(parameter_json,type_string);
	parameter_json = str_CatFree(parameter_json," ,\n");
	free(type_string);
	parameter_json = str_CatFree(parameter_json," \"value\" : ");
	switch(parameter->type)
	{
		case RAZER_PARAMETER_TYPE_STRING:
			{
				parameter_json = str_CatFree(parameter_json,"\"");
				parameter_json = str_CatFree(parameter_json,daemon_get_parameter_string(parameter));
				parameter_json = str_CatFree(parameter_json,"\" ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_INT:
			{
				char *int_string = str_FromLong(daemon_get_parameter_int(parameter));
				parameter_json = str_CatFree(parameter_json,int_string);
				free(int_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT:
			{
				char *float_string = str_FromDouble(daemon_get_parameter_float(parameter));
				parameter_json = str_CatFree(parameter_json,float_string);
				free(float_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB:
			{
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_rgb *color = daemon_get_parameter_rgb(parameter);
				char *r_string = str_FromLong(color->r);
				parameter_json = str_CatFree(parameter_json,r_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(r_string);
				char *g_string = str_FromLong(color->g);
				parameter_json = str_CatFree(parameter_json,g_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(g_string);
				char *b_string = str_FromLong(color->b);
				parameter_json = str_CatFree(parameter_json,b_string);
				free(b_string);
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
	}

	parameter_json = str_CatFree(parameter_json," \"description\": \"");
	parameter_json = str_CatFree(parameter_json,parameter->description);
	parameter_json = str_CatFree(parameter_json,"\" },\n");
	return(parameter_json);
}

struct razer_parameter *daemon_create_parameter(void)
{
	struct razer_parameter *parameter = (struct razer_parameter*)malloc(sizeof(struct razer_parameter));
	return(parameter);
}

void daemon_free_parameter(struct razer_parameter *parameter)
{
	if(parameter->key)
		free(parameter->key);
	if(parameter->description)
		free(parameter->description);
	//TODO free strings ?
	free(parameter);
}

void daemon_free_parameters(list *parameters)
{
	while(!list_IsEmpty(parameters))
	{
			daemon_free_parameter((struct razer_parameter*)list_Pop(parameters));
	}
	list_Close(parameters);
}

struct razer_parameter *daemon_copy_parameter(struct razer_parameter *parameter)
{
	struct razer_parameter *copy = daemon_create_parameter();
	copy->id = parameter->id;
	copy->key = str_Copy(parameter->key);
	copy->description = str_Copy(parameter->description);
	copy->type = parameter->type;
	switch(parameter->type)
	{
		case RAZER_PARAMETER_TYPE_STRING:
			daemon_set_parameter_string(copy,str_Copy(daemon_get_parameter_string(parameter)));
			break;
		case RAZER_PARAMETER_TYPE_INT:
			daemon_set_parameter_int(copy,daemon_get_parameter_int(parameter));
			break;
		case RAZER_PARAMETER_TYPE_FLOAT:
			daemon_set_parameter_float(copy,daemon_get_parameter_float(parameter));
			break;
		case RAZER_PARAMETER_TYPE_RGB:
			{
				struct razer_rgb *color = rgb_copy(daemon_get_parameter_rgb(parameter));
				daemon_set_parameter_rgb(copy,color);
			}
			break;
		case RAZER_PARAMETER_TYPE_POS:
			{
				struct razer_pos *pos = razer_pos_copy(daemon_get_parameter_pos(parameter));
				daemon_set_parameter_pos(copy,pos);
			}
			break;
		case RAZER_PARAMETER_TYPE_RENDER_NODE:
			daemon_set_parameter_render_node(copy,daemon_get_parameter_render_node(parameter));
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_RANGE:
			{
				struct razer_float_range *range = razer_float_range_copy(daemon_get_parameter_float_range(parameter));
				daemon_set_parameter_float_range(copy,range);
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_RANGE:
			{
				struct razer_int_range *range = razer_int_range_copy(daemon_get_parameter_int_range(parameter));
				daemon_set_parameter_int_range(copy,range);
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT_RANGE:
			{
				struct razer_uint_range *range = razer_uint_range_copy(daemon_get_parameter_uint_range(parameter));
				daemon_set_parameter_uint_range(copy,range);
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB_RANGE:
			{
				struct razer_rgb_range *range = razer_rgb_range_copy(daemon_get_parameter_rgb_range(parameter));
				daemon_set_parameter_rgb_range(copy,range);
			}
			break;
		case RAZER_PARAMETER_TYPE_POS_RANGE:
			{
				struct razer_pos_range *range = razer_pos_range_copy(daemon_get_parameter_pos_range(parameter));
				daemon_set_parameter_pos_range(copy,range);
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_ARRAY:
			{
				struct razer_float_array *array = razer_float_array_copy(daemon_get_parameter_float_array(parameter));
				daemon_set_parameter_float_array(copy,array);
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_ARRAY:
			{
				struct razer_int_array *array = razer_int_array_copy(daemon_get_parameter_int_array(parameter));
				daemon_set_parameter_int_array(copy,array);
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT_ARRAY:
			{
				struct razer_uint_array *array = razer_uint_array_copy(daemon_get_parameter_uint_array(parameter));
				daemon_set_parameter_uint_array(copy,array);
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB_ARRAY:
			{
				struct razer_rgb_array *array = razer_rgb_array_copy(daemon_get_parameter_rgb_array(parameter));
				daemon_set_parameter_rgb_array(copy,array);
			}
			break;
		case RAZER_PARAMETER_TYPE_POS_ARRAY:
			{
				struct razer_pos_array *array = razer_pos_array_copy(daemon_get_parameter_pos_array(parameter));
				daemon_set_parameter_pos_array(copy,array);
			}
			break;
	}
	
	return(copy);
}




//TODO maybe remove some redundancy here
struct razer_parameter *daemon_create_parameter_string(char *key,char *description,char *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_STRING;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_float(char *key,char *description,float value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	float *fp = (float*)&(parameter->value);
    *fp = value;
	parameter->type = RAZER_PARAMETER_TYPE_FLOAT;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_int(char *key,char *description,long value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_INT;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_uint(char *key,char *description,unsigned long value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = value;
	parameter->type = RAZER_PARAMETER_TYPE_UINT;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_rgb(char *key,char *description,struct razer_rgb *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RGB;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_pos(char *key,char *description,struct razer_pos *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_POS;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_render_node(char *key,char *description,struct razer_fx_render_node *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RENDER_NODE;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_float_range(char *key,char *description,struct razer_float_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_FLOAT_RANGE;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_int_range(char *key,char *description,struct razer_int_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_INT_RANGE;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_uint_range(char *key,char *description,struct razer_uint_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_UINT_RANGE;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_rgb_range(char *key,char *description,struct razer_rgb_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RGB_RANGE;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_pos_range(char *key,char *description,struct razer_pos_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_POS_RANGE;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_float_array(char *key,char *description,struct razer_float_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_FLOAT_ARRAY;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_int_array(char *key,char *description,struct razer_int_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_INT_ARRAY;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_uint_array(char *key,char *description,struct razer_uint_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_UINT_ARRAY;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_rgb_array(char *key,char *description,struct razer_rgb_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RGB_ARRAY;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_pos_array(char *key,char *description,struct razer_pos_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_POS_ARRAY;
	return(parameter);
}




void daemon_set_parameter_string(struct razer_parameter *parameter,char *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_STRING)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_float(struct razer_parameter *parameter,float value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_FLOAT)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	float *fp = (float*)&(parameter->value);
    *fp = value;
	//parameter->value = value;
}

void daemon_set_parameter_int(struct razer_parameter *parameter,long value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_INT)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_uint(struct razer_parameter *parameter,unsigned long value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_UINT)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = value;
}

void daemon_set_parameter_rgb(struct razer_parameter *parameter,struct razer_rgb *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_RGB)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_pos(struct razer_parameter *parameter,struct razer_pos *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_POS)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_render_node(struct razer_parameter *parameter,struct razer_fx_render_node *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_RENDER_NODE)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_float_range(struct razer_parameter *parameter,struct razer_float_range *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_FLOAT_RANGE)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_int_range(struct razer_parameter *parameter,struct razer_int_range *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_INT_RANGE)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_uint_range(struct razer_parameter *parameter,struct razer_uint_range *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_UINT_RANGE)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_rgb_range(struct razer_parameter *parameter,struct razer_rgb_range *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_RGB_RANGE)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_pos_range(struct razer_parameter *parameter,struct razer_pos_range *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_POS_RANGE)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_float_array(struct razer_parameter *parameter,struct razer_float_array *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_FLOAT_ARRAY)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_int_array(struct razer_parameter *parameter,struct razer_int_array *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_INT_ARRAY)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_uint_array(struct razer_parameter *parameter,struct razer_uint_array *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_UINT_ARRAY)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_rgb_array(struct razer_parameter *parameter,struct razer_rgb_array *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_RGB_ARRAY)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}

void daemon_set_parameter_pos_array(struct razer_parameter *parameter,struct razer_pos_array *value)
{
	if(parameter->type != RAZER_PARAMETER_TYPE_POS_ARRAY)
	{
		#ifdef USE_DEBUGGING
			printf("changing of parameter types is not allowed\n");
		#endif
		return;
	}
	parameter->value = (unsigned long)value;
}




char *daemon_get_parameter_string(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((char*)parameter->value);
}

float daemon_get_parameter_float(struct razer_parameter *parameter)
{
	if(!parameter)
		return(0.0f);
	float *fp = (float*)&(parameter->value);
	return(*fp);
	//return((float)parameter->value);
}

long daemon_get_parameter_int(struct razer_parameter *parameter)
{
	if(!parameter)
		return(0);
	return((long)parameter->value);
}

unsigned long daemon_get_parameter_uint(struct razer_parameter *parameter)
{
	if(!parameter)
		return(0);
	return(parameter->value);
}

struct razer_rgb *daemon_get_parameter_rgb(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_rgb*)parameter->value);
}

struct razer_pos *daemon_get_parameter_pos(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_pos*)parameter->value);
}

struct razer_fx_render_node *daemon_get_parameter_render_node(struct razer_parameter *parameter)
{
	return((struct razer_fx_render_node*)parameter->value);
}

struct razer_float_range *daemon_get_parameter_float_range(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_float_range*)parameter->value);
}

struct razer_int_range *daemon_get_parameter_int_range(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_int_range*)parameter->value);
}

struct razer_uint_range *daemon_get_parameter_uint_range(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_uint_range*)parameter->value);
}

struct razer_rgb_range *daemon_get_parameter_rgb_range(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_rgb_range*)parameter->value);
}

struct razer_pos_range *daemon_get_parameter_pos_range(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_pos_range*)parameter->value);
}

struct razer_float_array *daemon_get_parameter_float_array(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_float_array*)parameter->value);
}

struct razer_int_array *daemon_get_parameter_int_array(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_int_array*)parameter->value);
}

struct razer_uint_array *daemon_get_parameter_uint_array(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_uint_array*)parameter->value);
}

struct razer_rgb_array *daemon_get_parameter_rgb_array(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_rgb_array*)parameter->value);
}

struct razer_pos_array *daemon_get_parameter_pos_array(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_pos_array*)parameter->value);
}




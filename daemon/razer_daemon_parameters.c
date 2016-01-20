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
 #include "razer_daemon.h"


char *daemon_parameter_type_to_string(struct razer_parameter *parameter)
{
	switch(parameter->type)
	{
		case RAZER_PARAMETER_TYPE_STRING:
			return("String");
		case RAZER_PARAMETER_TYPE_INT:
			return("Integer");
		case RAZER_PARAMETER_TYPE_UINT:
			return("Unsigned Integer");
		case RAZER_PARAMETER_TYPE_FLOAT:
			return("Float");
		case RAZER_PARAMETER_TYPE_RGB:
			return("RGB");
		case RAZER_PARAMETER_TYPE_POS:
			return("Position");
		case RAZER_PARAMETER_TYPE_RENDER_NODE:
			return("Render Node");
		case RAZER_PARAMETER_TYPE_FLOAT_RANGE:
			return("Float Range");
		case RAZER_PARAMETER_TYPE_INT_RANGE:
			return("Integer Range");
		case RAZER_PARAMETER_TYPE_UINT_RANGE:
			return("Unsigned Integer Range");
		case RAZER_PARAMETER_TYPE_RGB_RANGE:
			return("RGB Range");
		case RAZER_PARAMETER_TYPE_POS_RANGE:
			return("Position Range");
		case RAZER_PARAMETER_TYPE_INT_ARRAY:
			return("Integer Array");
		case RAZER_PARAMETER_TYPE_UINT_ARRAY:
			return("Unsigned Integer Array");
		case RAZER_PARAMETER_TYPE_FLOAT_ARRAY:
			return("Float Array");
		case RAZER_PARAMETER_TYPE_RGB_ARRAY:
			return("RGB Array");
		case RAZER_PARAMETER_TYPE_POS_ARRAY:
			return("Position Array");
		default:
			return("Unknown Type");
	}
}




char *daemon_parameter_to_json(struct razer_parameter *parameter, int final)
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
	parameter_json = str_CatFree(parameter_json," \"type\" : \"");
	//char *type_string = str_FromLong(parameter->type);
	//parameter_json = str_CatFree(parameter_json,type_string);
	parameter_json = str_CatFree(parameter_json,daemon_parameter_type_to_string(parameter));
	parameter_json = str_CatFree(parameter_json,"\" ,\n");
	//free(type_string);
	parameter_json = str_CatFree(parameter_json," \"private\" : ");
	if(parameter->private)
		parameter_json = str_CatFree(parameter_json," true,\n");
	else
		parameter_json = str_CatFree(parameter_json," false,\n");


	switch(parameter->type)
	{
		case RAZER_PARAMETER_TYPE_STRING:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				parameter_json = str_CatFree(parameter_json,"\"");
				parameter_json = str_CatFree(parameter_json,daemon_get_parameter_string(parameter));
				parameter_json = str_CatFree(parameter_json,"\" ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_INT:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				char *int_string = str_FromLong(daemon_get_parameter_int(parameter));
				parameter_json = str_CatFree(parameter_json,int_string);
				free(int_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				char *int_string = str_FromLong(daemon_get_parameter_int(parameter));
				parameter_json = str_CatFree(parameter_json,int_string);
				free(int_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				char *float_string = str_FromDouble(daemon_get_parameter_float(parameter));
				parameter_json = str_CatFree(parameter_json,float_string);
				free(float_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
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
		case RAZER_PARAMETER_TYPE_POS:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_pos *pos = daemon_get_parameter_pos(parameter);
				char *x_string = str_FromLong(pos->x);
				parameter_json = str_CatFree(parameter_json,x_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(x_string);
				char *y_string = str_FromLong(pos->y);
				parameter_json = str_CatFree(parameter_json,y_string);
				free(y_string);
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_RENDER_NODE:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				struct razer_fx_render_node *render_node = daemon_get_parameter_render_node(parameter);
				char *render_node_string = str_FromLong(render_node->id);
				parameter_json = str_CatFree(parameter_json,render_node_string);
				free(render_node_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_RANGE:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_float_range *range = daemon_get_parameter_float_range(parameter);
				char *min_string = str_FromDouble(range->min);
				parameter_json = str_CatFree(parameter_json,min_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(min_string);
				char *max_string = str_FromDouble(range->max);
				parameter_json = str_CatFree(parameter_json,max_string);
				free(max_string);
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_RANGE:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_int_range *range = daemon_get_parameter_int_range(parameter);
				char *min_string = str_FromLong(range->min);
				parameter_json = str_CatFree(parameter_json,min_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(min_string);
				char *max_string = str_FromLong(range->max);
				parameter_json = str_CatFree(parameter_json,max_string);
				free(max_string);
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT_RANGE:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_uint_range *range = daemon_get_parameter_uint_range(parameter);
				char *min_string = str_FromLong(range->min);
				parameter_json = str_CatFree(parameter_json,min_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(min_string);
				char *max_string = str_FromLong(range->max);
				parameter_json = str_CatFree(parameter_json,max_string);
				free(max_string);
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB_RANGE:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_rgb_range *range = daemon_get_parameter_rgb_range(parameter);
				struct razer_rgb *min = range->min;
				parameter_json = str_CatFree(parameter_json,"{\n\"R\" : ");
				char *min_r_string = str_FromLong(min->r);
				parameter_json = str_CatFree(parameter_json,min_r_string);
				parameter_json = str_CatFree(parameter_json," ,\n\"G\" : ");
				free(min_r_string);
				char *min_g_string = str_FromLong(min->g);
				parameter_json = str_CatFree(parameter_json,min_g_string);
				parameter_json = str_CatFree(parameter_json," ,\n\"B\" : ");
				free(min_g_string);
				char *min_b_string = str_FromLong(min->b);
				parameter_json = str_CatFree(parameter_json,min_b_string);
				free(min_b_string);
				parameter_json = str_CatFree(parameter_json,"} ,\n");
				parameter_json = str_CatFree(parameter_json," ,");
				parameter_json = str_CatFree(parameter_json,"{");
				struct razer_rgb *max = range->max;
				parameter_json = str_CatFree(parameter_json,"{\n\"R\" : ");
				char *max_r_string = str_FromLong(max->r);
				parameter_json = str_CatFree(parameter_json,max_r_string);
				parameter_json = str_CatFree(parameter_json," ,\n\"G\" : ");
				free(max_r_string);
				char *max_g_string = str_FromLong(max->g);
				parameter_json = str_CatFree(parameter_json,max_g_string);
				parameter_json = str_CatFree(parameter_json," ,\n\"B\" : ");
				free(max_g_string);
				char *max_b_string = str_FromLong(max->b);
				parameter_json = str_CatFree(parameter_json,max_b_string);
				free(max_b_string);
				parameter_json = str_CatFree(parameter_json,"} ,\n");
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_POS_RANGE:
			{
				parameter_json = str_CatFree(parameter_json," \"value\" : ");
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_pos_range *range = daemon_get_parameter_pos_range(parameter);
				struct razer_pos *min = range->min;
				parameter_json = str_CatFree(parameter_json,"{\n\"X\" : ");
				char *min_x_string = str_FromLong(min->x);
				parameter_json = str_CatFree(parameter_json,min_x_string);
				parameter_json = str_CatFree(parameter_json," ,\n\"Y\" : ");
				free(min_x_string);
				char *min_y_string = str_FromLong(min->y);
				parameter_json = str_CatFree(parameter_json,min_y_string);
				free(min_y_string);
				parameter_json = str_CatFree(parameter_json,"} ,\n");
				parameter_json = str_CatFree(parameter_json," ,");
				parameter_json = str_CatFree(parameter_json,"{");
				struct razer_pos *max = range->max;
				parameter_json = str_CatFree(parameter_json,"{\n\"X\" : ");
				char *max_x_string = str_FromLong(max->x);
				parameter_json = str_CatFree(parameter_json,max_x_string);
				parameter_json = str_CatFree(parameter_json," ,\n\"Y\" : ");
				free(max_x_string);
				char *max_y_string = str_FromLong(max->y);
				parameter_json = str_CatFree(parameter_json,max_y_string);
				free(max_y_string);
				parameter_json = str_CatFree(parameter_json,"} ,\n");
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
		/*case RAZER_PARAMETER_TYPE_INT_ARRAY:
		case RAZER_PARAMETER_TYPE_UINT_ARRAY:
		case RAZER_PARAMETER_TYPE_FLOAT_ARRAY:
		case RAZER_PARAMETER_TYPE_RGB_ARRAY:
		case RAZER_PARAMETER_TYPE_POS_ARRAY:
		*/
	}

	parameter_json = str_CatFree(parameter_json," \"description\": \"");
	parameter_json = str_CatFree(parameter_json,parameter->description);
	if(final)
	{
		parameter_json = str_CatFree(parameter_json,"\" }\n");
	} else {
		parameter_json = str_CatFree(parameter_json,"\" },\n");
	}

	return(parameter_json);
}

char *daemon_parameter_array_to_json(struct razer_parameter *parameter,int array_index)
{
	char *parameter_json = str_CreateEmpty();
	parameter_json = str_CatFree(parameter_json,"{\n \"item\": \"");
	parameter_json = str_CatFree(parameter_json,parameter->key);
	parameter_json = str_CatFree(parameter_json,"\",\n");
	parameter_json = str_CatFree(parameter_json," \"id\" : ");
	char *id_string = str_FromLong(array_index);
	parameter_json = str_CatFree(parameter_json,id_string);
	parameter_json = str_CatFree(parameter_json," ,\n");
	free(id_string);
	parameter_json = str_CatFree(parameter_json," \"value\" : ");
	struct razer_array_header *header = (struct razer_array_header*)parameter->value;
	if(array_index<0 || array_index >= header->size)
	{
		parameter_json = str_CatFree(parameter_json," null \n},\n");
		return(parameter_json);
	}
	switch(parameter->type)
	{
		case RAZER_PARAMETER_TYPE_POS_ARRAY:
			{
				struct razer_pos *pos = ((struct razer_pos_array*)parameter->value)->values[array_index];
				parameter_json = str_CatFree(parameter_json,"[");
				char *x_string = str_FromLong(pos->x);
				parameter_json = str_CatFree(parameter_json,x_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(x_string);
				char *y_string = str_FromLong(pos->y);
				parameter_json = str_CatFree(parameter_json,y_string);
				parameter_json = str_CatFree(parameter_json," ,");
				free(y_string);
				parameter_json = str_CatFree(parameter_json,"] ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT_ARRAY:
			{
				unsigned long value = ((struct razer_uint_array*)parameter->value)->values[array_index];
				char *uint_string = str_FromLong(value);
				parameter_json = str_CatFree(parameter_json,uint_string);
				free(uint_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_ARRAY:
			{
				long value = ((struct razer_int_array*)parameter->value)->values[array_index];
				char *int_string = str_FromLong(value);
				parameter_json = str_CatFree(parameter_json,int_string);
				free(int_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_ARRAY:
			{
				float value = ((struct razer_float_array*)parameter->value)->values[array_index];
				char *float_string = str_FromDouble((double)value);
				parameter_json = str_CatFree(parameter_json,float_string);
				parameter_json = str_CatFree(parameter_json," ,\n");
				free(float_string);
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB_ARRAY:
			{
				parameter_json = str_CatFree(parameter_json,"[");
				struct razer_rgb *color = ((struct razer_rgb_array*)parameter->value)->values[array_index];
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

	parameter_json = str_CatFree(parameter_json," },\n");
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
	//parameter->effect = NULL;
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
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_int(char *key,char *description,long value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_INT;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_uint(char *key,char *description,unsigned long value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = value;
	parameter->type = RAZER_PARAMETER_TYPE_UINT;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_rgb(char *key,char *description,struct razer_rgb *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RGB;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_pos(char *key,char *description,struct razer_pos *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_POS;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_render_node(char *key,char *description,struct razer_fx_render_node *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RENDER_NODE;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_float_range(char *key,char *description,struct razer_float_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_FLOAT_RANGE;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_int_range(char *key,char *description,struct razer_int_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_INT_RANGE;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_uint_range(char *key,char *description,struct razer_uint_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_UINT_RANGE;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_rgb_range(char *key,char *description,struct razer_rgb_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RGB_RANGE;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_pos_range(char *key,char *description,struct razer_pos_range *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_POS_RANGE;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_float_array(char *key,char *description,struct razer_float_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_FLOAT_ARRAY;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_int_array(char *key,char *description,struct razer_int_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_INT_ARRAY;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_uint_array(char *key,char *description,struct razer_uint_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_UINT_ARRAY;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_rgb_array(char *key,char *description,struct razer_rgb_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RGB_ARRAY;
	//parameter->effect = NULL;
	return(parameter);
}

struct razer_parameter *daemon_create_parameter_pos_array(char *key,char *description,struct razer_pos_array *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_POS_ARRAY;
	//parameter->effect = NULL;
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




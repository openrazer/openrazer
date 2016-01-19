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
#include "razer_chroma_controller.h"

#ifdef USE_DBUS


int dc_dbus_error_check(char*message,DBusError *error)
{
	if(dbus_error_is_set(error))
	{
		#ifdef USE_DEBUGGING
		printf("dbus (%s) error:%s\n",message,error->message);
		#endif
		dbus_error_free(error);
		return(0);
	}
	return(1);
}

int dc_dbus_open(struct razer_daemon_controller *controller)
{
	DBusError error;
	dbus_error_init(&error);
	controller->dbus = dbus_bus_get(DBUS_BUS_SYSTEM,&error);
	if(!dc_dbus_error_check("open",&error))
		return(0);
	if(!controller->dbus)
		return(0);
	return(1);
}

void dc_dbus_close(struct razer_daemon_controller *controller)
{
	if(controller->dbus)
		dbus_connection_unref(controller->dbus);
}

struct razer_daemon_controller *dc_open(void)
{
	struct razer_daemon_controller *controller = (struct razer_daemon_controller*)malloc(sizeof(struct razer_daemon_controller));
	#ifdef USE_DBUS
	 	controller->dbus = NULL;
	 	if(!dc_dbus_open(controller))
	 	{
	 		free(controller);
			return(NULL);
		}
		#ifdef USE_VERBOSE_DEBUGGING
			printf("dbus: opened\n");
		#endif
	#endif
 	return(controller);
}

void dc_close(struct razer_daemon_controller *controller)
{
	#ifdef USE_DBUS
		dc_dbus_close(controller);
		#ifdef USE_VERBOSE_DEBUGGING
			printf("dbus: closed\n");
		#endif
	#endif
 	free(controller);
}

/*int dc_is_daemon_running(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon","is_paused");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		return(0);
	return(1);
}*/

void dc_error_close(struct razer_daemon_controller *controller,char *message)
{
	printf("daemon controller error: %s\n",message);
	#ifdef USE_DBUS
		dc_dbus_close(controller);
	#endif
 	free(controller);
 	exit(1);
}

void dc_quit(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon","quit");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_continue(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon","continue");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_pause(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon","pause");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

int dc_render_node_create(struct razer_daemon_controller *controller,int effect_uid,int device_uid,char *name,char *description)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.render_node","create");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&effect_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&device_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&name))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&description))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	int render_node_uid = -1;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_INT32) 
		dc_error_close(controller,"Argument is not an int!\n"); 
	else
		dbus_message_iter_get_basic(&args,&render_node_uid);
	dbus_message_unref(msg);   
	return(render_node_uid);
}

void dc_render_node_reset(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	//DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node","reset");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_default_render_node_set(struct razer_daemon_controller *controller,int device_uid,int render_node_uid) 
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *duid = str_FromLong(device_uid);
	path = str_CatFree(path,duid);
	free(duid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node","set");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&render_node_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}


char *dc_render_node_parameter_get(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid,int array_index)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	path = str_CatFree(path,"/");
	char *puid = str_FromLong(parameter_uid);
	path = str_CatFree(path,puid);
	free(puid);
	if(array_index!=-1)
	{
		path = str_CatFree(path,"/");
		char *aid = str_FromLong(array_index);
		path = str_CatFree(path,aid);
		free(aid);
	}
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.parameter","get");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	char *parameter_json = NULL;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) 
		dc_error_close(controller,"Argument is not a string!\n"); 
	else
		dbus_message_iter_get_basic(&args,&parameter_json);
	dbus_message_unref(msg);   
	parameter_json = str_Copy(parameter_json);
	free(path);//TODO gets not freed on error
	return(parameter_json);
}


int dc_parameter_type_from_string(char *type)
{
	if(!strcasecmp(type,"Str"))
		return(RAZER_PARAMETER_TYPE_STRING);
	else if(!strcasecmp(type,"Int"))
		return(RAZER_PARAMETER_TYPE_INT);
	else if(!strcasecmp(type,"UInt"))
		return(RAZER_PARAMETER_TYPE_UINT);
	else if(!strcasecmp(type,"Float"))
		return(RAZER_PARAMETER_TYPE_FLOAT);
	else if(!strcasecmp(type,"RGB"))
		return(RAZER_PARAMETER_TYPE_RGB);
	else if(!strcasecmp(type,"Pos"))
		return(RAZER_PARAMETER_TYPE_POS);
	else if(!strcasecmp(type,"Node"))
		return(RAZER_PARAMETER_TYPE_RENDER_NODE);
	else if(!strcasecmp(type,"String"))
		return(RAZER_PARAMETER_TYPE_STRING);
	else if(!strcasecmp(type,"Float_Range"))
		return(RAZER_PARAMETER_TYPE_FLOAT_RANGE);
	else if(!strcasecmp(type,"Int_Range"))
		return(RAZER_PARAMETER_TYPE_INT_RANGE);
	else if(!strcasecmp(type,"UInt_Range"))
		return(RAZER_PARAMETER_TYPE_UINT_RANGE);
	else if(!strcasecmp(type,"RGB_Range"))
		return(RAZER_PARAMETER_TYPE_RGB_RANGE);
	else if(!strcasecmp(type,"Pos_Range"))
		return(RAZER_PARAMETER_TYPE_POS_RANGE);
	else if(!strcasecmp(type,"Int_Array"))
		return(RAZER_PARAMETER_TYPE_INT_ARRAY);
	else if(!strcasecmp(type,"UInt_Array"))
		return(RAZER_PARAMETER_TYPE_UINT_ARRAY);
	else if(!strcasecmp(type,"Float_Array"))
		return(RAZER_PARAMETER_TYPE_FLOAT_ARRAY);
	else if(!strcasecmp(type,"RGB_Array"))
		return(RAZER_PARAMETER_TYPE_RGB_ARRAY);
	else if(!strcasecmp(type,"Pos_Array"))
		return(RAZER_PARAMETER_TYPE_POS_ARRAY);
	else 
		return(RAZER_PARAMETER_TYPE_UNKNOWN);
}


int dc_render_node_parameter_parsed_set(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid,int array_index,char *type,char *value_string)
{
	int itype = dc_parameter_type_from_string(type);
	//printf("type found:%d\n",itype);
	unsigned long long value = 0;
	int free_value_after = 0;
	switch(itype)
	{
		case RAZER_PARAMETER_TYPE_STRING:
			value=(unsigned long long)value_string;
			break;
		case RAZER_PARAMETER_TYPE_INT:
			value = atol(value_string);
			break;
		case RAZER_PARAMETER_TYPE_UINT:
			value = atol(value_string);
			break;
		case RAZER_PARAMETER_TYPE_FLOAT:
			value = atof(value_string);
			break;
		case RAZER_PARAMETER_TYPE_RGB:
		 	{
				value = (unsigned long long)malloc(sizeof(struct razer_rgb));
				list *tokens = list_Create(3,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 3)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_rgb*)value)->r = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb*)value)->g = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb*)value)->b = (unsigned char)atoi(list_Dequeue(tokens));
				//printf("got 3 integers:%d,%d,%d\n",((struct razer_rgb*)value)->r,((struct razer_rgb*)value)->g,((struct razer_rgb*)value)->b);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_POS:
			{
				value = (unsigned long long)malloc(sizeof(struct razer_pos));
				list *tokens = list_Create(2,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 2)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_pos*)value)->x = (long)atol(list_Dequeue(tokens));
				((struct razer_pos*)value)->y = (long)atol(list_Dequeue(tokens));
				//printf("got 2 integers:%d,%d\n",((struct razer_pos*)value)->x,((struct razer_pos*)value)->y);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_RENDER_NODE:
			value = atol(value_string);
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_RANGE:
			{
				value = (unsigned long long)malloc(sizeof(struct razer_float_range));
				list *tokens = list_Create(2,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 2)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_float_range*)value)->min = (float)atof(list_Dequeue(tokens));
				((struct razer_float_range*)value)->max = (float)atof(list_Dequeue(tokens));
				//printf("got 2 floats:%f,%f\n",((struct razer_float_range*)value)->min,((struct razer_float_range*)value)->max);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_RANGE:
			{
				value = (unsigned long long)malloc(sizeof(struct razer_int_range));
				list *tokens = list_Create(2,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 2)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_int_range*)value)->min = (long)atol(list_Dequeue(tokens));
				((struct razer_int_range*)value)->max = (long)atol(list_Dequeue(tokens));
				//printf("got 2 integers:%d,%d\n",((struct razer_int_range*)value)->min,((struct razer_int_range*)value)->max);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT_RANGE:
			{
				value = (unsigned long long)malloc(sizeof(struct razer_uint_range));
				list *tokens = list_Create(2,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 2)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_uint_range*)value)->min = (unsigned long)atol(list_Dequeue(tokens));
				((struct razer_uint_range*)value)->max = (unsigned long)atol(list_Dequeue(tokens));
				//printf("got 2 unsigned integers:%d,%d\n",((struct razer_uint_range*)value)->min,((struct razer_uint_range*)value)->max);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB_RANGE:
			{
				value = (unsigned long long)malloc(sizeof(struct razer_rgb_range));
				list *tokens = list_Create(6,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 6)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_rgb_range*)value)->min->r = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb_range*)value)->min->g = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb_range*)value)->min->b = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb_range*)value)->max->r = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb_range*)value)->max->g = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb_range*)value)->max->b = (unsigned char)atoi(list_Dequeue(tokens));
				//printf("got 6 integers:%d,%d,%d,%d,%d,%d\n",((struct razer_rgb_range*)value)->min->r,((struct razer_rgb_range*)value)->min->g,((struct razer_rgb_range*)value)->min->b,((struct razer_rgb_range*)value)->max->r,((struct razer_rgb_range*)value)->max->g,((struct razer_rgb_range*)value)->max->b);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_POS_RANGE:
			{
				value = (unsigned long long)malloc(sizeof(struct razer_pos_range));
				list *tokens = list_Create(4,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 4)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_pos_range*)value)->min->x = (long)atol(list_Dequeue(tokens));
				((struct razer_pos_range*)value)->min->y = (long)atol(list_Dequeue(tokens));
				((struct razer_pos_range*)value)->max->x = (long)atol(list_Dequeue(tokens));
				((struct razer_pos_range*)value)->max->y = (long)atol(list_Dequeue(tokens));
				//printf("got 4 integers:%d,%d,%d,%d\n",((struct razer_pos_range*)value)->min->x,((struct razer_pos_range*)value)->min->y,((struct razer_pos_range*)value)->max->x,((struct razer_pos_range*)value)->max->y);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_ARRAY: //TODO i know its redundant here.maybe remove later
			value = atol(value_string);
			break;
		case RAZER_PARAMETER_TYPE_UINT_ARRAY:
			value = atol(value_string);
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_ARRAY:
			value = atof(value_string);
			break;
		case RAZER_PARAMETER_TYPE_RGB_ARRAY:
		 	{
				value = (unsigned long long)malloc(sizeof(struct razer_rgb));
				list *tokens = list_Create(3,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 3)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_rgb*)value)->r = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb*)value)->g = (unsigned char)atoi(list_Dequeue(tokens));
				((struct razer_rgb*)value)->b = (unsigned char)atoi(list_Dequeue(tokens));
				//printf("got 3 integers:%d,%d,%d\n",((struct razer_rgb*)value)->r,((struct razer_rgb*)value)->g,((struct razer_rgb*)value)->b);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_POS_ARRAY:
			{
				value = (unsigned long long)malloc(sizeof(struct razer_pos));
				list *tokens = list_Create(2,0);
				long items = str_Tokenize(value_string," ",&tokens);
				if(items != 2)
				{
					str_FreeTokens(tokens);
					break;
				}
				((struct razer_pos*)value)->x = (long)atol(list_Dequeue(tokens));
				((struct razer_pos*)value)->y = (long)atol(list_Dequeue(tokens));
				//printf("got 2 integers:%d,%d\n",((struct razer_pos*)value)->x,((struct razer_pos*)value)->y);
				str_FreeTokens(tokens);
				free_value_after = 1;
			}
			break;
		case RAZER_PARAMETER_TYPE_UNKNOWN:
			return(0);
	}
	dc_render_node_parameter_set(controller,render_node_uid,parameter_uid,array_index,itype,value);
	if(free_value_after)
		free((void*)value);
	return(1);
}


void dc_render_node_parameter_set(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid,int array_index,int type,unsigned long long value)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	path = str_CatFree(path,"/");
	char *puid = str_FromLong(parameter_uid);
	path = str_CatFree(path,puid);
	free(puid);
	if(array_index!=-1)
	{
		path = str_CatFree(path,"/");
		char *aid = str_FromLong(array_index);
		path = str_CatFree(path,aid);
		free(aid);
	}
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.parameter","set");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	
	switch(type)
	{
		case RAZER_PARAMETER_TYPE_STRING:
			if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&value))
				dc_error_close(controller,"Out of memory!\n"); 
			break;
		case RAZER_PARAMETER_TYPE_INT:
			{
				long l = (long)value;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&l))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT:
			{
				double f = (double)value;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_DOUBLE,&f))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_RENDER_NODE:
			{
				long rn_uid = (long)value;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&rn_uid))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT:
			{
				unsigned long ul = (unsigned long)value;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_UINT32,&ul))
					dc_error_close(controller,"Out of memory!\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB:
			{
				long r = ((struct razer_rgb*)value)->r;
				long g = ((struct razer_rgb*)value)->g;
				long b = ((struct razer_rgb*)value)->b;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&r))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&g))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&b))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_POS:
			{	
				long x = ((struct razer_pos*)value)->x;
				long y = ((struct razer_pos*)value)->y;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&x))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&y))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_RANGE:
			{
				double min = ((struct razer_float_range*)value)->min;
				double max = ((struct razer_float_range*)value)->max;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_DOUBLE,&min))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_DOUBLE,&max))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_RANGE:
			{
				long min = ((struct razer_int_range*)value)->min;
				long max = ((struct razer_int_range*)value)->max;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&min))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&max))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT_RANGE:
			{
				unsigned long min = ((struct razer_uint_range*)value)->min;
				unsigned long max = ((struct razer_uint_range*)value)->max;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_UINT32,&min))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_UINT32,&max))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB_RANGE:
			{
				long min_r = ((struct razer_rgb*)(((struct razer_rgb_range*)value)->min))->r;
				long min_g = ((struct razer_rgb*)(((struct razer_rgb_range*)value)->min))->g;
				long min_b = ((struct razer_rgb*)(((struct razer_rgb_range*)value)->min))->b;
				long max_r = ((struct razer_rgb*)(((struct razer_rgb_range*)value)->max))->r;
				long max_g = ((struct razer_rgb*)(((struct razer_rgb_range*)value)->max))->g;
				long max_b = ((struct razer_rgb*)(((struct razer_rgb_range*)value)->max))->b;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&min_r))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&min_g))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&min_b))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&max_r))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&max_g))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&max_b))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_POS_RANGE:
			{
				long min_x = ((struct razer_pos*)(((struct razer_pos_range*)value)->min))->x;
				long min_y = ((struct razer_pos*)(((struct razer_pos_range*)value)->min))->y;
				long max_x = ((struct razer_pos*)(((struct razer_pos_range*)value)->max))->x;
				long max_y = ((struct razer_pos*)(((struct razer_pos_range*)value)->max))->y;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&min_x))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&min_y))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&max_x))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&max_y))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_INT_ARRAY:
			{
				long l = (long)value;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&l))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_UINT_ARRAY:
			{
				unsigned long ul = (unsigned long)value;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_UINT32,&ul))
					dc_error_close(controller,"Out of memory!\n");
			}
			break;
		case RAZER_PARAMETER_TYPE_FLOAT_ARRAY:
			{
				double f = (double)value;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_DOUBLE,&f))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_RGB_ARRAY:
			{
				long r = ((struct razer_rgb*)value)->r;
				long g = ((struct razer_rgb*)value)->g;
				long b = ((struct razer_rgb*)value)->b;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&r))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&g))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&b))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		case RAZER_PARAMETER_TYPE_POS_ARRAY:
			{	
				long x = ((struct razer_pos*)value)->x;
				long y = ((struct razer_pos*)value)->y;
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&x))
					dc_error_close(controller,"Out of memory!\n"); 
				if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&y))
					dc_error_close(controller,"Out of memory!\n"); 
			}
			break;
		default:
			printf("Error unknown type :%d\n",type);
			free(path);
			return;
	}

	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}


float dc_render_node_opacity_get(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.opacity","get");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	double opacity = -1.0f;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_DOUBLE) 
		dc_error_close(controller,"Argument is not an float!\n"); 
	else
		dbus_message_iter_get_basic(&args,&opacity);
	dbus_message_unref(msg);   
	free(path);//TODO gets not freed on error
	return((float)opacity);
}

void dc_render_node_opacity_set(struct razer_daemon_controller *controller,int render_node_uid,float opacity)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	double opc = (double)opacity;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.opacity","set");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_DOUBLE,&opc))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}

void dc_render_node_input_connect(struct razer_daemon_controller *controller,int render_node_uid,int input_render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.input","connect");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&input_render_node_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}

void dc_render_node_second_input_connect(struct razer_daemon_controller *controller,int render_node_uid,int second_input_render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.second_input","connect");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&second_input_render_node_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}

int dc_render_node_next_get(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.next","get");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	int next_uid = -1;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_INT32) 
		dc_error_close(controller,"Argument is not an int!\n"); 
	else
		dbus_message_iter_get_basic(&args,&next_uid);
	dbus_message_unref(msg);   
	free(path);//TODO gets not freed on error
	return(next_uid);
}

void dc_render_node_next_set(struct razer_daemon_controller *controller,int render_node_uid,int next_render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.next","set");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&next_render_node_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}

int dc_render_node_next_move_frame_buffer_linkage_get(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage","get");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	int move_linkage = -1;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_INT32) 
		dc_error_close(controller,"Argument is not an int!\n"); 
	else
		dbus_message_iter_get_basic(&args,&move_linkage);
	dbus_message_unref(msg);   
	free(path);//TODO gets not freed on error
	return(move_linkage);
}

void dc_render_node_next_move_frame_buffer_linkage_set(struct razer_daemon_controller *controller,int render_node_uid,int move_linkage)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage","set");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&move_linkage))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}

int dc_render_node_parent_get(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.parent","get");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	int parent_uid = -1;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_INT32) 
		dc_error_close(controller,"Argument is not an int!\n"); 
	else
		dbus_message_iter_get_basic(&args,&parent_uid);
	dbus_message_unref(msg);   
	free(path);//TODO gets not freed on error
	return(parent_uid);
}

void dc_render_node_limit_render_time_ms_set(struct razer_daemon_controller *controller,int render_node_uid,int limit_ms)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.limit_render_time_ms","set");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&limit_ms))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);
}

void dc_render_node_sub_add(struct razer_daemon_controller *controller,int render_node_uid,int sub_render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.sub","add");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&sub_render_node_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);
}

void dc_frame_buffer_connect(struct razer_daemon_controller *controller,int device_uid,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *duid = str_FromLong(device_uid);
	path = str_CatFree(path,duid);
	free(duid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.device.frame_buffer","connect");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&render_node_uid))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}

void dc_frame_buffer_disconnect(struct razer_daemon_controller *controller,int device_uid)
{
	DBusMessage *msg;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *duid = str_FromLong(device_uid);
	path = str_CatFree(path,duid);
	free(duid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemondevice.frame_buffer","disconnect");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
	free(path);//TODO gets not freed on error
}

char *dc_fx_list(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.fx","list");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	char *list = NULL;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) 
		dc_error_close(controller,"Argument is not a string!\n"); 
	else
		dbus_message_iter_get_basic(&args,&list);
	//if(!dbus_message_iter_next(&args))
	//	dc_error_close(controller,"Message has too few arguments!\n"); 
	//else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UINT32) 
	//	dc_error_close(controller,"Argument is not int!\n"); 
	//else
	//	dbus_message_iter_get_basic(&args,&level);
	//printf("fx List: %s\n",list);
	list = str_Copy(list);
	dbus_message_unref(msg);   
	return(list);
}

char *dc_render_nodes_list(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.render_nodes","list");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	char *list = NULL;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) 
		dc_error_close(controller,"Argument is not a string!\n"); 
	else
		dbus_message_iter_get_basic(&args,&list);
	//if(!dbus_message_iter_next(&args))
	//	dc_error_close(controller,"Message has too few arguments!\n"); 
	//else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UINT32) 
	//	dc_error_close(controller,"Argument is not int!\n"); 
	//else
	//	dbus_message_iter_get_basic(&args,&level);
	//printf("fx List: %s\n",list);
	list = str_Copy(list);
	dbus_message_unref(msg);   
	return(list);
}

char *dc_rendering_nodes_list(struct razer_daemon_controller *controller,int device_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *duid = str_FromLong(device_uid);
	path = str_CatFree(path,duid);
	free(duid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.device.render_nodes","render_list");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	char *list = NULL;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) 
		dc_error_close(controller,"Argument is not a string!\n"); 
	else
		dbus_message_iter_get_basic(&args,&list);
	//if(!dbus_message_iter_next(&args))
	//	dc_error_close(controller,"Message has too few arguments!\n"); 
	//else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UINT32) 
	//	dc_error_close(controller,"Argument is not int!\n"); 
	//else
	//	dbus_message_iter_get_basic(&args,&level);
	//printf("fx List: %s\n",list);
	list = str_Copy(list);
	dbus_message_unref(msg);   
	free(path);//TODO gets not freed on error
	return(list);
}

char *dc_sub_nodes_list(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.subs","list");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	char *list = NULL;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) 
		dc_error_close(controller,"Argument is not a string!\n"); 
	else
		dbus_message_iter_get_basic(&args,&list);
	//if(!dbus_message_iter_next(&args))
	//	dc_error_close(controller,"Message has too few arguments!\n"); 
	//else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UINT32) 
	//	dc_error_close(controller,"Argument is not int!\n"); 
	//else
	//	dbus_message_iter_get_basic(&args,&level);
	//printf("fx List: %s\n",list);
	list = str_Copy(list);
	dbus_message_unref(msg);   
	return(list);
}

char *dc_render_node_parameters_list(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *suid = str_FromLong(render_node_uid);
	path = str_CatFree(path,suid);
	free(suid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.render_node.parameters","list");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	char *list = NULL;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) 
		dc_error_close(controller,"Argument is not a string!\n"); 
	else
		dbus_message_iter_get_basic(&args,&list);
	//if(!dbus_message_iter_next(&args))
	//	dc_error_close(controller,"Message has too few arguments!\n"); 
	//else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UINT32) 
	//	dc_error_close(controller,"Argument is not int!\n"); 
	//else
	//	dbus_message_iter_get_basic(&args,&level);
	//printf("fx List: %s\n",list);
	list = str_Copy(list);
	dbus_message_unref(msg);   
	return(list);
}

int dc_is_paused(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon","is_paused");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	int is_paused = -1;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_INT32) 
		dc_error_close(controller,"Argument is not an int!\n"); 
	else
		dbus_message_iter_get_basic(&args,&is_paused);
	dbus_message_unref(msg);   
	return(is_paused);
}

void dc_fps_set(struct razer_daemon_controller *controller,int fps)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.fps","set");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&fps))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

int dc_fps_get(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.fps","get");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	int fps = -1;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_INT32) 
		dc_error_close(controller,"Argument is not an int!\n"); 
	else
		dbus_message_iter_get_basic(&args,&fps);
	dbus_message_unref(msg);   
	return(fps);
}

int dc_frame_buffer_get(struct razer_daemon_controller *controller,int device_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	char *path = str_CreateEmpty();
	path = str_CatFree(path,"/");
	char *duid = str_FromLong(device_uid);
	path = str_CatFree(path,duid);
	free(duid);
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon",path,"org.voyagerproject.razer.daemon.device.frame_buffer","get");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);

	int uid = -1;

	dbus_pending_call_block(controller->pending);
	msg = dbus_pending_call_steal_reply(controller->pending);
	if(!msg)
		dc_error_close(controller,"Empty reply\n"); 
	dbus_pending_call_unref(controller->pending);
	if(!dbus_message_iter_init(msg,&args))
		dc_error_close(controller,"Message has no arguments!\n"); 
	else if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_INT32) 
		dc_error_close(controller,"Argument is not an int!\n"); 
	else
		dbus_message_iter_get_basic(&args,&uid);
	dbus_message_unref(msg);   
	free(path);//TODO gets not freed on error
	return(uid);
}

void dc_load_fx_lib(struct razer_daemon_controller *controller,char *fx_pathname)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.fx.lib","load");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_STRING,&fx_pathname))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_set_spectrum_mode(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.driver_effect","spectrum");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_set_wave_mode(struct razer_daemon_controller *controller, unsigned char direction)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.driver_effect","wave");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&direction))
		dc_error_close(controller,"Out of memory!\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_set_reactive_mode(struct razer_daemon_controller *controller, unsigned char speed, unsigned char red, unsigned char green, unsigned char blue)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.driver_effect","reactive");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&speed))
			dc_error_close(controller,"Out of memory!\n");
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&red))
		dc_error_close(controller,"Out of memory!\n");
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&green))
			dc_error_close(controller,"Out of memory!\n");
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&blue))
			dc_error_close(controller,"Out of memory!\n");

	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_set_breath_mode(struct razer_daemon_controller *controller, unsigned char breath_type, unsigned char red, unsigned char green, unsigned char blue, unsigned char red2, unsigned char green2, unsigned char blue2) // ??
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.driver_effect","breath");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");

	if(breath_type == 1 || breath_type == 2)
	{
		dbus_message_iter_init_append(msg,&args);
		if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&red))
			dc_error_close(controller,"Out of memory!\n");
		if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&green))
			dc_error_close(controller,"Out of memory!\n");
		if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&blue))
			dc_error_close(controller,"Out of memory!\n");
	}
	if(breath_type == 2)
	{
		dbus_message_iter_init_append(msg,&args);
		if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&red2))
			dc_error_close(controller,"Out of memory!\n");
		if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&green2))
			dc_error_close(controller,"Out of memory!\n");
		if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&blue2))
			dc_error_close(controller,"Out of memory!\n");
	}

	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_set_static_mode(struct razer_daemon_controller *controller, unsigned char red, unsigned char green, unsigned char blue)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.driver_effect","static");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&red))
		dc_error_close(controller,"Out of memory!\n");
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&green))
			dc_error_close(controller,"Out of memory!\n");
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&blue))
			dc_error_close(controller,"Out of memory!\n");

	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_set_none_mode(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.driver_effect","none");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_set_keyboard_brightness(struct razer_daemon_controller *controller, unsigned char brightness)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon","raw_keyboard_brightness");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_BYTE,&brightness))
		dc_error_close(controller,"Out of memory!\n");

	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_enable_macro_keys(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon","enable_macro_keys");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n");
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n");
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

/*
int dc_create_render_node(struct razer_daemon_controller *controller,int effect_uid,int input_render_node_uid,int second_input_render_node_uid,int output_render_node_uid,char *name,char *description)
{
}

struct razer_effect *dc_get_effect(struct razer_daemon_controller *controller,int effect_uid)
{
}

struct razer_fx_render_node *dc_get_render_node(struct razer_daemon_controller *controller,int render_node_uid)
{
}
*/

//end of dbus ifdef
#endif 

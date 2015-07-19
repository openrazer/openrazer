#include "razer_daemon.h"

void daemon_kill(struct razer_daemon *daemon,char *error_message)
{
	daemon->running = 0;
	printf("Exiting daemon.\nError: %s\n",error_message);
	exit(1);
}



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

char *daemon_effect_to_json(struct razer_effect *effect)
{
	char *effect_json = str_CreateEmpty();
	effect_json = str_CatFree(effect_json,"\n {\n \"name\": \"");
	effect_json = str_CatFree(effect_json,effect->name);
	effect_json = str_CatFree(effect_json,"\" ,\n");
	effect_json = str_CatFree(effect_json," \"id\" : ");
	char *id_string = str_FromLong(effect->id);
	effect_json = str_CatFree(effect_json,id_string);
	effect_json = str_CatFree(effect_json," ,\n");
	free(id_string);
	effect_json = str_CatFree(effect_json," \"description\": \"");
	effect_json = str_CatFree(effect_json,effect->description);
	effect_json = str_CatFree(effect_json,"\" ,\n \"parameters_num\" : ");
	char *parameters_num_string = str_FromLong(effect->parameters->num);
	effect_json = str_CatFree(effect_json,parameters_num_string);
	effect_json = str_CatFree(effect_json," ,\n");
	free(parameters_num_string);
	effect_json = str_CatFree(effect_json," \"parameters\" :  [\n");
	for(int i = 0;i<effect->parameters->num;i++)
	{
		struct razer_parameter *parameter = effect->parameters->items[i];
		char *parameter_json = daemon_parameter_to_json(parameter);
		effect_json = str_CatFree(effect_json,parameter_json);
		free(parameter_json);
	}
	effect_json = str_CatFree(effect_json,"] },\n");
	return(effect_json);
}

char *daemon_render_node_to_json(struct razer_fx_render_node *render_node)
{
	char *rn_json = str_CreateEmpty();
	rn_json = str_CatFree(rn_json,"\n {\n");
	rn_json = str_CatFree(rn_json," \"id\" : ");
	char *id_string = str_FromLong(render_node->id);
	rn_json = str_CatFree(rn_json,id_string);
	rn_json = str_CatFree(rn_json," ,\n");
	free(id_string);
	rn_json = str_CatFree(rn_json," \"name\": \"");
	rn_json = str_CatFree(rn_json,render_node->name);
	rn_json = str_CatFree(rn_json,"\" ,\ndescription\": \"");
	rn_json = str_CatFree(rn_json,render_node->description);
	/*effect_json = str_CatFree(effect_json,"\" ,\n \"subs_num\" : ");
	char *parameters_num_string = str_FromLong(effect->parameters->num);
	effect_json = str_CatFree(effect_json,parameters_num_string);
	effect_json = str_CatFree(effect_json," ,\n");
	free(parameters_num_string);
	effect_json = str_CatFree(effect_json," \"parameters\" :  [\n");
	for(int i = 0;i<effect->parameters->num;i++)
	{
		struct razer_parameter *parameter = effect->parameters->items[i];
		char *parameter_json = daemon_parameter_to_json(parameter);
		effect_json = str_CatFree(effect_json,parameter_json);
		free(parameter_json);
	}*/
	rn_json = str_CatFree(rn_json,"\"\n },\n");
	return(rn_json);
}

#ifdef USE_DBUS

int daemon_dbus_error_check(char*message,DBusError *error)
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

int daemon_dbus_open(struct razer_daemon *daemon)
{
	DBusError error;
	dbus_error_init(&error);
	daemon->dbus = dbus_bus_get(DBUS_BUS_SESSION,&error);
	if(!daemon_dbus_error_check("open",&error))
		return(0);
	if(!daemon->dbus)
		return(0);
	return(1);
}

int daemon_dbus_add_method(struct razer_daemon *daemon,char *interface_name, char *method_name)
{
	//cat interface_name + method_name
	DBusError error;
	dbus_error_init(&error);
	char *match_front = "type='method_call',interface='";
	char *match = str_CreateEmpty();
	match = str_CatFree(match,match_front);
	match = str_CatFree(match,interface_name);
	match = str_CatFree(match,".");
	match = str_CatFree(match,method_name);
	match = str_CatFree(match,"'");
	#ifdef USE_VERBOSE_DEBUGGING
	printf("adding dbus method_call: %s\n",match);
	#endif
	dbus_bus_add_match(daemon->dbus,match,&error);
	free(match);
	if(!daemon_dbus_error_check("add_match",&error))
		return(0);
	return(1);
}

int daemon_dbus_announce(struct razer_daemon *daemon)
{
	DBusError error;
	dbus_error_init(&error);
	dbus_bus_request_name(daemon->dbus,"org.voyagerproject.razer.daemon",DBUS_NAME_FLAG_REPLACE_EXISTING,&error);
	if(!daemon_dbus_error_check("request_name",&error))
		return(0);
	//if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fx","set"))
	//	return(0);
	//if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fx","get"))
	//	return(0);
	//if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fx.instances","list"))
	//	return(0);
	//if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fx.libs","list"))
	//	return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fps","set"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fps","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fx","list"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.fx.lib","load"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_nodes","list"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_nodes","render_list"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node","create"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node","set")) //TODO Remove 
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.opacity","set"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.opacity","get")) 
		return(0);
	//if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.output","connect"))
	//	return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.input","connect"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.second_input","connect"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.next","set"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.next","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parent","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.subs","add"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.subs","list"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parameters","list"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parameter","set"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parameter","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.frame_buffer","connect"))
		return(0);
	return(1);
}

int daemon_dbus_get_string_array_len(char **path)
{
	int i = 0;
	while(path[i]!=NULL)
		i++;
	return(i);
}


int daemon_dbus_handle_messages(struct razer_daemon *daemon)
{
	DBusMessage *msg;
	DBusMessage *reply;
	DBusMessageIter parameters;
	dbus_connection_read_write(daemon->dbus,0);
	msg = dbus_connection_pop_message(daemon->dbus);
	if(!msg)
		return(0);
	//printf("dbus: received message:type:%d ,path:%s ,interface:%s ,member:%s\n",dbus_message_get_type(msg),dbus_message_get_path(msg),dbus_message_get_interface(msg),dbus_message_get_member(msg));
    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fps", "set")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set fps called\n");
		#endif
		if(!dbus_message_iter_init(msg, &parameters))
		{
			#ifdef USE_DEBUGGING
				printf("dbus: set fps received without parameter\n");
			#endif
		}
		if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
		{
			dbus_message_iter_get_basic(&parameters,&daemon->render_node->effect->fps);
			#ifdef USE_DEBUGGING
				printf("parameter:%d\n",daemon->render_node->effect->fps);
			#endif
		}
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&daemon->render_node->effect->fps)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fps", "get")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get fps called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&daemon->render_node->effect->fps)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fx", "list")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list fx called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *fx_list_json = str_CreateEmpty();
		fx_list_json = str_CatFree(fx_list_json,"{\n");
		fx_list_json = str_CatFree(fx_list_json," \"effects_num\" : ");
		char *effects_num_string = str_FromLong(daemon->effects_num);
		fx_list_json = str_CatFree(fx_list_json,effects_num_string);
		fx_list_json = str_CatFree(fx_list_json," ,\n");
		free(effects_num_string);
		fx_list_json = str_CatFree(fx_list_json," \"effects_list\": [\n");
		for(int i=0;i<daemon->effects_num;i++)
		{
			struct razer_effect *effect = daemon->effects[i];
			char *effect_json = daemon_effect_to_json(effect);
			fx_list_json = str_CatFree(fx_list_json,effect_json);
			free(effect_json);
		}
		fx_list_json = str_CatFree(fx_list_json,"]}\n");
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&fx_list_json)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(fx_list_json);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_nodes", "list")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list render_nodes called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *rn_list_json = str_CreateEmpty();
		rn_list_json = str_CatFree(rn_list_json,"{\n");
		rn_list_json = str_CatFree(rn_list_json," \"render_nodes_num\" : ");
		char *rn_num_string = str_FromLong(daemon->fx_render_nodes_num);
		rn_list_json = str_CatFree(rn_list_json,rn_num_string);
		rn_list_json = str_CatFree(rn_list_json," ,\n");
		free(rn_num_string);
		rn_list_json = str_CatFree(rn_list_json," \"render_nodes_list\": [\n");
		for(int i=0;i<daemon->fx_render_nodes_num;i++)
		{
			struct razer_fx_render_node *render_node = daemon->fx_render_nodes[i];
			char *rn_json = daemon_render_node_to_json(render_node);
			rn_list_json = str_CatFree(rn_list_json,rn_json);
			free(rn_json);
		}
		rn_list_json = str_CatFree(rn_list_json,"]}\n");
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&rn_list_json)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(rn_list_json);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_nodes", "render_list")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list rendering render_nodes called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *rn_list_json = str_CreateEmpty();
		rn_list_json = str_CatFree(rn_list_json,"{\n");
		rn_list_json = str_CatFree(rn_list_json," \"render_nodes_num\" : ");
		char *rn_num_string = str_FromLong(daemon->render_nodes_num);
		rn_list_json = str_CatFree(rn_list_json,rn_num_string);
		rn_list_json = str_CatFree(rn_list_json," ,\n");
		free(rn_num_string);
		rn_list_json = str_CatFree(rn_list_json," \"render_nodes_list\": [\n");
		for(int i=0;i<daemon->render_nodes_num;i++)
		{
			struct razer_fx_render_node *render_node = daemon->render_nodes[i];
			char *rn_json = daemon_render_node_to_json(render_node);
			rn_list_json = str_CatFree(rn_list_json,rn_json);
			free(rn_json);
		}
		rn_list_json = str_CatFree(rn_list_json,"]}\n");
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&rn_list_json)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(rn_list_json);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parameters", "list")) 
	{
		char **path = NULL;
		int rn_uid = -1;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list render_nodes.parameters called for : %s\n",path[0]);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		char *rn_list_json = str_CreateEmpty();
		dbus_free_string_array(path);
		struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
		if(render_node)
		{
			rn_list_json = str_CatFree(rn_list_json,"{\n");
			rn_list_json = str_CatFree(rn_list_json," \"parameters_num\" : ");
			char *rn_num_string = str_FromLong(render_node->effect->parameters->num);
			rn_list_json = str_CatFree(rn_list_json,rn_num_string);
			rn_list_json = str_CatFree(rn_list_json," ,\n");
			free(rn_num_string);
			rn_list_json = str_CatFree(rn_list_json," \"parameters_list\": [\n");
			for(int i=0;i<render_node->effect->parameters->num;i++)
			{
				char *rn_json = daemon_parameter_to_json(render_node->effect->parameters->items[i]);
				rn_list_json = str_CatFree(rn_list_json,rn_json);
				free(rn_json);
			}
			rn_list_json = str_CatFree(rn_list_json,"]}\n");
		}
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&rn_list_json)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(rn_list_json);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parameter", "get"))
	{
		char **path = NULL;
		int rn_uid = -1;
		int p_index = -1;
		dbus_message_get_path_decomposed(msg,&path);
		int path_len = daemon_dbus_get_string_array_len(path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get render_nodes.parameter called for : %d\n",path_len);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path_len == 2)
		{
			rn_uid = atoi(path[0]);
			p_index = atoi(path[1]);
			#ifdef USE_DEBUGGING
				printf("dbus: get parameter path : %d , %d\n",rn_uid,p_index);
				printf("dbus: get parameter max path render_node : %d\n",daemon->fx_render_nodes_num);
			#endif
		}
		char *rn_list_json = str_CreateEmpty();
		dbus_free_string_array(path);
		if(daemon->fx_render_nodes_num)
		{
			//struct razer_fx_render_node *render_node = daemon->fx_render_nodes[rn_uid];
			struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
			if(render_node && p_index>=0 && p_index < render_node->effect->parameters->num)
			{
				struct razer_parameter *parameter = render_node->effect->parameters->items[p_index];
				char *parameter_json = daemon_parameter_to_json(parameter);
				rn_list_json = str_CatFree(rn_list_json,parameter_json);
				free(parameter_json);
			}
		}
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&rn_list_json)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(rn_list_json);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parameter", "set"))
	{
		char **path = NULL;
		int rn_uid = -1;
		int p_index = -1;
		dbus_message_get_path_decomposed(msg,&path);
		int path_len = daemon_dbus_get_string_array_len(path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set render_nodes.parameter called for : %d\n",path_len);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path_len == 2)
		{
			rn_uid = atoi(path[0]);
			p_index = atoi(path[1]);
			#ifdef USE_DEBUGGING
				printf("dbus: set parameter path : %d , %d\n",rn_uid,p_index);
				printf("dbus: set parameter max path render_node : %d\n",daemon->fx_render_nodes_num);
			#endif
		}
		dbus_free_string_array(path);
		if(dbus_message_iter_init(msg, &parameters) && daemon->fx_render_nodes_num)
		{
			//struct razer_fx_render_node *render_node = daemon->fx_render_nodes[rn_uid];
			struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
			
			if(render_node && p_index>=0 && p_index < render_node->effect->parameters->num)
			{
				struct razer_parameter *parameter = render_node->effect->parameters->items[p_index];
				switch(parameter->type)
				{
					case RAZER_PARAMETER_TYPE_STRING:
						if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
						{
							char *fxname;
							dbus_message_iter_get_basic(&parameters,&fxname);
							#ifdef USE_DEBUGGING
								printf("dbus: string parameter:%s\n",fxname);
							#endif
						}
						break;
					case RAZER_PARAMETER_TYPE_FLOAT:
						if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_DOUBLE)
						{
							double value = 0;
							dbus_message_iter_get_basic(&parameters,&value);
							#ifdef USE_DEBUGGING
								printf("dbus: float parameter:%d\n",value);
							#endif
							daemon_set_parameter_float(parameter,value);
						}
						break;
					case RAZER_PARAMETER_TYPE_INT:
						if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
						{
							long value = 0;
							dbus_message_iter_get_basic(&parameters,&value);
							#ifdef USE_DEBUGGING
								printf("dbus: int parameter:%d\n",value);
							#endif
							daemon_set_parameter_int(parameter,value);
						}
						break;
					case RAZER_PARAMETER_TYPE_RGB:
						{
							struct razer_rgb *color = daemon_get_parameter_rgb(parameter);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: R parameter:%d\n",value);
								#endif
								color->r = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: G parameter:%d\n",value);
								#endif
								color->g = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: B parameter:%d\n",value);
								#endif
								color->b = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_RENDER_NODE:
						break;
				}
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node", "create"))
	{
		int fx_uid=0;
		char *name = NULL;
		char *description = NULL;
		reply = dbus_message_new_method_return(msg);
		#ifdef USE_DEBUGGING
			printf("\ndbus: creating render_node\n");
		#endif

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&fx_uid);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
			{
				dbus_message_iter_get_basic(&parameters,&name);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
			{
				dbus_message_iter_get_basic(&parameters,&description);
			}

			if(fx_uid && description && name)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_create_render_node(daemon,daemon_get_effect(daemon,fx_uid),-1,-1,-1,name,description);
				daemon_register_render_node(daemon,rn);
				char *rn_uid_json = str_CreateEmpty();
				rn_uid_json = str_CatFree(rn_uid_json,"{\n");
				rn_uid_json = str_CatFree(rn_uid_json," \"uid\" : ");
				char *rn_uid_string = str_FromLong(rn->id);
				rn_uid_json = str_CatFree(rn_uid_json,rn_uid_string);
				rn_uid_json = str_CatFree(rn_uid_json,"\n");
				free(rn_uid_string);
				rn_uid_json = str_CatFree(rn_uid_json,"}\n");
				if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&rn_uid_json)) 
					daemon_kill(daemon,"dbus: Out Of Memory!\n");
				free(rn_uid_json);
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node", "set"))
	{
		int rn_uid=0;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&rn_uid);
			}
			if(rn_uid)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				#ifdef USE_DEBUGGING
					printf("\ndbus: setting render_node to: %d (@%x)\n",rn_uid,rn);
					//fflush(stdout);
				#endif
				if(rn)
					daemon->render_node = rn;
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.frame_buffer", "connect"))
	{
		int rn_uid=0;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&rn_uid);
			}
			if(rn_uid)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				#ifdef USE_DEBUGGING
					printf("\ndbus: connecting output frame buffer to render_node to: %d (@%x)\n",rn_uid,rn);
				#endif
				daemon_connect_frame_buffer(daemon,rn);
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.opacity", "set"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set render_node.opacity called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		double opacity=-1.0f;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_DOUBLE)
			{
				dbus_message_iter_get_basic(&parameters,&opacity);
			}
			if(rn_uid && opacity != -1.0f)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				rn->opacity = opacity;
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.opacity", "get"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get render_node.opacity called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(rn_uid)
		{
			dbus_message_iter_init_append(reply,&parameters);
			struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
			double opacity = rn->opacity;
			if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_DOUBLE,&opacity)) 
				daemon_kill(daemon,"dbus: Out Of Memory!\n");
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.input", "connect"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method connect render_node.input called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		int dst_rn_uid=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&dst_rn_uid);
			}
			if(rn_uid && dst_rn_uid)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				struct razer_fx_render_node *dst_rn = daemon_get_render_node(daemon,dst_rn_uid);
				#ifdef USE_DEBUGGING
					printf("dbus: connecting input frame buffer to render_node : %d (@%x) to %d (@%x)\n",rn_uid,rn,dst_rn_uid,dst_rn);
				#endif
				daemon_connect_input(daemon,rn,dst_rn);
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.second_input", "connect"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method connect render_node.second_input called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		int dst_rn_uid=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&dst_rn_uid);
			}
			if(rn_uid && dst_rn_uid)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				struct razer_fx_render_node *dst_rn = daemon_get_render_node(daemon,dst_rn_uid);
				#ifdef USE_DEBUGGING
					printf("dbus: connecting second input frame buffer to render_node : %d (@%x) to %d (@%x)\n",rn_uid,rn,dst_rn_uid,dst_rn);
				#endif
				daemon_connect_second_input(daemon,rn,dst_rn);
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.next", "set"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set render_node.next called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		int dst_rn_uid=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&dst_rn_uid);
			}
			if(rn_uid && dst_rn_uid)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				struct razer_fx_render_node *dst_rn = daemon_get_render_node(daemon,dst_rn_uid);
				rn->next = dst_rn;
				#ifdef USE_DEBUGGING
				if(dst_rn->prev)
					printf("Warning: Render_node.prev already set.\n");
				#endif
				dst_rn->prev = rn;
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.next", "get"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get render_node.next called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(rn_uid)
		{
			dbus_message_iter_init_append(reply,&parameters);
			struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
			if(rn->next)
			{
				int next_uid = rn->next->id;
				if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&next_uid)) 
					daemon_kill(daemon,"dbus: Out Of Memory!\n");
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parent", "get"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get render_node.parent called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(rn_uid)
		{
			dbus_message_iter_init_append(reply,&parameters);
			struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
			if(rn->parent)
			{
				int parent_uid = rn->parent->id;
				if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&parent_uid)) 
					daemon_kill(daemon,"dbus: Out Of Memory!\n");
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.limit_render_time_ms", "set"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set render_node.limit_render_time_ms called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		int limit_render_time_ms=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&limit_render_time_ms);
			}
			if(rn_uid && limit_render_time_ms)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				rn->limit_render_time_ms = limit_render_time_ms;
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.subs", "list")) 
	{
		char **path = NULL;
		int rn_uid = -1;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list render_nodes.subs called for : %s\n",path[0]);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		char *rn_list_json = str_CreateEmpty();
		dbus_free_string_array(path);
		struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
		if(render_node)
		{
			rn_list_json = str_CatFree(rn_list_json,"{\n");
			rn_list_json = str_CatFree(rn_list_json," \"subs_num\" : ");
			char *rn_num_string = str_FromLong(render_node->subs_num);
			rn_list_json = str_CatFree(rn_list_json,rn_num_string);
			free(rn_num_string);
			if(render_node->subs_num)
			{
				rn_list_json = str_CatFree(rn_list_json," ,\n");
				rn_list_json = str_CatFree(rn_list_json," \"subs_list\": [\n");
				for(int i=0;i<render_node->subs_num;i++)
				{
					char *rn_json = daemon_render_node_to_json(render_node->subs[i]);
					rn_list_json = str_CatFree(rn_list_json,rn_json);
					free(rn_json);
				}
				rn_list_json = str_CatFree(rn_list_json,"]}\n");
			}
			else
				rn_list_json = str_CatFree(rn_list_json,"}\n");
		}
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&rn_list_json)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(rn_list_json);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.sub", "add"))
	{
		char **path = NULL;
		int rn_uid = -1;
		int s_uid = -1;
		dbus_message_get_path_decomposed(msg,&path);
		int path_len = daemon_dbus_get_string_array_len(path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method add render_nodes.sub called for : %d\n",path_len);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path_len == 1)
		{
			rn_uid = atoi(path[0]);
			#ifdef USE_DEBUGGING
				printf("dbus: add sub path : %d\n",rn_uid);
			#endif
		}
		dbus_free_string_array(path);
		if(dbus_message_iter_init(msg, &parameters) && daemon->fx_render_nodes_num)
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&s_uid);
			}
			if(s_uid)
			{
				struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
				struct razer_fx_render_node *sub_render_node = daemon_get_render_node(daemon,s_uid);
				if(render_node && sub_render_node)
				{
					daemon_render_node_add_sub(render_node,sub_render_node);
				}
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fx.lib", "load"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method load fx.lib called for\n");
		#endif
		char *lib_filename = NULL;
		reply = dbus_message_new_method_return(msg);
		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
			{
				dbus_message_iter_get_basic(&parameters,&lib_filename);
			}

			if(lib_filename)
			{
				void *lib = daemon_load_fx_lib(daemon,lib_filename);
				if(lib)
				{
					daemon_register_lib(daemon,lib);
				}
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	/*
    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fx.libs.", "list")) 
	{
		#ifdef USE_DEBUGGING
			printf("dbus: method list fx libs called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *libs_list_json = str_CreateEmpty();
		libs_list_json = str_CatFree(libs_list_json,"{\n");
		libs_list_json = str_CatFree(libs_list_json," \"libs_num\" : ");
		char *libs_num_string = str_FromLong(daemon->libs_num);
		libs_list_json = str_CatFree(libs_list_json,libs_num_string);
		libs_list_json = str_CatFree(libs_list_json," ,\n");
		free(libs_num_string);
		libs_list_json = str_CatFree(libs_list_json," \"libs_list\": [\n");
		for(int i=0;i<daemon->libs_num;i++)
		{
			struct razer_effect *effect = daemon->effects[i];
			char *effect_json = daemon_effect_to_json(effect);
			libs_list_json = str_CatFree(libs_list_json,effect_json);
			free(effect_json);
		}
		libs_list_json = str_CatFree(libs_list_json,"]}\n");
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&libs_list_json)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(libs_list_json);
	}
	*/



    if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fx", "set")) 
	{
		#ifdef USE_DEBUGGING
			printf("dbus: method set fx called\n");
		#endif
		if(!dbus_message_iter_init(msg, &parameters))
		{
			#ifdef USE_DEBUGGING
				printf("dbus: signal set fx received without parameter\n");
			#endif
		}
		if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
		{
			char *fxname;
			dbus_message_iter_get_basic(&parameters,&fxname);
			printf("parameter:%s\n",fxname);
		}

		struct razer_rgb col={.r=5,.g=0,.b=0};
		for(int i=0;i<250;i++)
		{
			col.r = 5+(i*1);
			set_all(daemon->chroma->keys,&col);
			razer_update_keys(daemon->chroma,daemon->chroma->keys);
			//usleep((1000*1000)/255);
		}
		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}

    if(dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect")) 
	{
		#ifdef USE_DEBUGGING
			printf("dbus: method Introspect called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *xml_data = 
		"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n\
		<node>\n\
		<interface name=\"org.freedesktop.DBus.Introspectable\">\n\
		<method name=\"Introspect\">\n\
		<arg direction=\"out\" name=\"data\" type=\"s\">\n\
		</arg></method>\n\
		</interface>\n\
		<interface name=\"org.voyagerproject.razer.daemon.fx\">\n\
		<method name=\"set\">\n\
		<arg direction=\"in\" name=\"fxname\" type=\"s\">\n\
		</arg></method></interface>\n\
		<interface name=\"org.freedesktop.DBus.Properties\">\n\
		<method name=\"Get\">\n\
		<arg direction=\"in\" name=\"interface\" type=\"s\">\n\
		<arg direction=\"in\" name=\"propname\" type=\"s\">\n\
		<arg direction=\"out\" name=\"value\" type=\"v\">\n\
		</arg></arg></arg></method>\n\
		<method name=\"Set\">\n\
		<arg direction=\"in\" name=\"interface\" type=\"s\">\n\
		<arg direction=\"in\" name=\"propname\" type=\"s\">\n\
		<arg direction=\"in\" name=\"value\" type=\"v\">\n\
		</arg></arg></arg></method>\n\
		<method name=\"GetAll\">\n\
		<arg direction=\"in\" name=\"interface\" type=\"s\">\n\
		<arg direction=\"out\" name=\"props\" type=\"a{sv}\">\n\
		</arg></arg></method>\n\
		</interface>\n\
		</node>\n";
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&xml_data)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}


	if(dbus_message_is_signal(msg,"org.voyagerproject.razer.daemon.fx.set.Type","set"))
	{
		#ifdef USE_DEBUGGING
			printf("dbus: signal set fx received\n");
		#endif
		if(!dbus_message_iter_init(msg, &parameters))
		{
			#ifdef USE_DEBUGGING
				printf("dbus: signal set fx received without parameter\n");
			#endif
		}
		if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
		{
			char *fxname;
			dbus_message_iter_get_basic(&parameters,&fxname);
			printf("parameter:%s\n",fxname);
		}
	}
	dbus_message_unref(msg);
	return(1);
}

void daemon_connect_frame_buffer(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(daemon->frame_buffer_linked_uid != 0) //unlink old render node first
	{
		struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
		old_rn->output_frame = daemon_create_rgb_frame();
		old_rn->output_frame_linked_uid = -1;
	}
	if(render_node->output_frame_linked_uid == -1)
		daemon_free_rgb_frame(&render_node->output_frame);
	render_node->output_frame = daemon->frame_buffer;
	daemon->frame_buffer_linked_uid = render_node->id;
	daemon->is_render_nodes_dirty = 1;
}

void daemon_connect_input(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node)
{
	//if(render_node->input_frame_linked_uid != 0) //unlink old render node first
	//{
	//	struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
	//	old_rn->output_frame = daemon_create_rgb_frame();
	//	old_rn->output_frame_linked_uid = -1;
	//}
	if(render_node->input_frame_linked_uid == -1)
		daemon_free_rgb_frame(&render_node->input_frame);
	render_node->input_frame = input_node->output_frame;
	render_node->input_frame_linked_uid = input_node->id;
	daemon->is_render_nodes_dirty = 1;
}

void daemon_connect_second_input(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,struct razer_fx_render_node *input_node)
{
	//if(render_node->input_frame_linked_uid != 0) //unlink old render node first
	//{
	//	struct razer_fx_render_node *old_rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
	//	old_rn->output_frame = daemon_create_rgb_frame();
	//	old_rn->output_frame_linked_uid = -1;
	//}
	if(render_node->second_input_frame_linked_uid == -1)
		daemon_free_rgb_frame(&render_node->second_input_frame);
	render_node->second_input_frame = input_node->output_frame;
	render_node->second_input_frame_linked_uid = input_node->id;
	daemon->is_render_nodes_dirty = 1;
}

void daemon_set_default_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	daemon->render_node = render_node;
}


void daemon_dbus_close(struct razer_daemon *daemon)
{
	if(daemon->dbus)
		dbus_connection_unref(daemon->dbus);
}

#endif


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

void *daemon_load_fx_lib(struct razer_daemon *daemon,char *fx_pathname)
{
	#ifdef USE_DEBUGGING
		printf("loading fx lib: %s\n",fx_pathname);
	#endif
	void *lib = dlopen(fx_pathname,RTLD_LAZY);
	if(lib==NULL)
	{
		#ifdef USE_DEBUGGING
			printf("fx lib not loaded:%s (%s)\n",fx_pathname,dlerror());
		#endif
		return(NULL);
	} 
	else
	{
		void *sym = dlsym(lib,"fx_init");
		if(sym==NULL)
		{
			#ifdef USE_DEBUGGING
				printf("no init function found\n");
			#endif
		    dlclose(lib);
		    return(NULL);
		}
		else
		{
			razer_effect_init init = (razer_effect_init)sym;
			init(daemon);
		}
	}
	return(lib);
}

#pragma GCC diagnostic pop

//int daemon_unload_fx_lib(struct razer_daemon *daemon,)

struct razer_daemon *daemon_open(void)
{
 	//signal(SIGINT,stop);
 	//signal(SIGKILL,stop);
    //signal(SIGTERM,stop);	
	struct razer_daemon *daemon = (struct razer_daemon*)malloc(sizeof(struct razer_daemon));
 	daemon->chroma = (struct razer_chroma*)malloc(sizeof(struct razer_chroma));
 	daemon->running = 1;
 	daemon->fps = 12;
 	daemon->libs_num = 0;
 	daemon->libs = NULL;
 	daemon->effects_uid = 1;
 	daemon->effects_num = 0;
 	daemon->effects = NULL;
 	daemon->fx_render_nodes_num = 0;
 	daemon->fx_render_nodes_uid = 1;
 	daemon->fx_render_nodes = NULL;//list of all render_nodes available
 	daemon->is_render_nodes_dirty = 0;
 	daemon->render_nodes = NULL;
 	daemon->render_nodes_num = 0;

 	if(!razer_open(daemon->chroma))
 	{
 		free(daemon->chroma);
 		free(daemon);
		return(NULL);
	}
	#ifdef USE_DBUS
	 	daemon->dbus = NULL;
		#ifdef USE_DEBUGGING
			printf("dbus: opened\n");
		#endif
	 	if(!daemon_dbus_open(daemon))
	 	{
	 		free(daemon->chroma);
	 		free(daemon);
			return(NULL);
		}
	 	if(!daemon_dbus_announce(daemon))
	 	{
 			free(daemon->chroma);
	 		free(daemon);
			return(NULL);
		}
	#endif
	razer_set_input_handler(daemon->chroma,daemon_key_event_handler);
	daemon->chroma->tag = daemon;
	daemon->frame_buffer = daemon_create_rgb_frame();
	daemon->frame_buffer_linked_uid = 0;
	daemon->return_render_node = NULL;

	razer_set_custom_mode(daemon->chroma);
	clear_all(daemon->chroma->keys);
	razer_update_keys(daemon->chroma,daemon->chroma->keys);

	#ifdef USE_DEBUGGING
		void *lib = daemon_load_fx_lib(daemon,"daemon/fx/pez2001_collection_debug.so");
	#else
		//void *lib = daemon_load_fx_lib(daemon,"daemon/fx/pez2001_collection.so");
		void *lib = daemon_load_fx_lib(daemon,"/usr/share/razer_bcd/fx/pez2001_collection.so");
	#endif
	if(lib)
		daemon_register_lib(daemon,lib);

	daemon->render_node = daemon_create_render_node(daemon,daemon_get_effect(daemon,2),-1,-1,0,"First Test Render Node","Default Render Node");
	daemon_register_render_node(daemon,daemon->render_node);
	daemon_compute_render_nodes(daemon);
	daemon_connect_frame_buffer(daemon,daemon->render_node);

	/*daemon->render_node = daemon_create_render_node(daemon,daemon_get_effect(daemon,3),-1,-1,0,"Second Test Render Node","Additional Testing Render Node");
	daemon_register_render_node(daemon,daemon->render_node);
	daemon_compute_render_nodes(daemon);
	*/
 	return(daemon);
}

void daemon_close(struct razer_daemon **daemon)
{
	#ifdef USE_DBUS
		daemon_dbus_close((*daemon));
	#endif
 	razer_close((*daemon)->chroma);
 	free((*daemon)->chroma);
 	free(*daemon);
}

//needed to be able to use different parameter sets for multiple copies of the same effect in der render_nodes chain
struct razer_effect *daemon_create_effect_instance(struct razer_daemon *daemon,struct razer_effect *lib_effect)
{
	//create an instance of a base effect housed in external libs
	//create own instances of each razer_parameter included
	//return the instance
	struct razer_effect *instance = daemon_create_effect();
	instance->id = daemon->effects_uid++;
	instance->name = str_Copy(lib_effect->name);
	instance->description = str_Copy(lib_effect->description);
	instance->fps = lib_effect->fps;
	instance->open = lib_effect->open;
	instance->close = lib_effect->close;
	instance->update = lib_effect->update;
	instance->key_event = lib_effect->key_event;
	instance->dbus_event = lib_effect->dbus_event;
	for(int i=0;i<lib_effect->parameters->num;i++)
	{
		daemon_add_parameter(instance->parameters,daemon_copy_parameter(lib_effect->parameters->items[i]));
	}
	return(instance);
}

int daemon_has_render_node_reached_render_limit(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(render_node->limit_render_time_ms)
	{
		//printf("checking render_limit %d : %u > %u + %u (%u)\n",render_node->id,daemon->chroma->update_ms,render_node->start_ticks,render_node->limit_render_time_ms,render_node->start_ticks + render_node->limit_render_time_ms);
		if(daemon->chroma->update_ms > (render_node->start_ticks + render_node->limit_render_time_ms))
		{
			#ifdef USE_DEBUGGING
				printf("\nreached render limit for node: %d\n",render_node->id);
			#endif
			return(1);
		}
	}
	return(0);
}

int daemon_update_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(!render_node || !render_node->effect)
		return(-1);

	if(!render_node->start_ticks)
	{
		render_node->start_ticks = razer_get_ticks();
		render_node->running  = 1;
		for(int i=0;i<render_node->subs_num;i++)
		{
			struct razer_fx_render_node *sub = render_node->subs[i];
			sub->start_ticks = 0;
			sub->running  = 1;
		}
	}
	if(!render_node->running)
		return(0);
	if(render_node->subs_num)
	{
		/*#ifdef USE_DEBUGGING
			printf("## has compute nodes ##");
		#endif*/
		for(int i=0;i<render_node->subs_num;i++)
		{
			struct razer_fx_render_node *sub = render_node->subs[i];
			if(!sub->effect)
				continue;
			if(sub->effect->class&=2)
				continue;//only execute compute effects
			if(!sub->start_ticks)
			{
				sub->start_ticks = razer_get_ticks();
				sub->running  = 1;
			}
			if(!sub->running)
				continue;
			int sub_ret = daemon_update_render_node(daemon,sub);
			if(!sub_ret || daemon_has_render_node_reached_render_limit(daemon,sub) || !sub->running)
			{
				if(sub->next)
				{
					render_node->subs[i] = sub->next;
					sub->next->parent = render_node;
					sub->next->start_ticks = 0; 
				}
				sub->running = 0;
				//return(0);
			}
		}
	}
	if(!render_node->effect->update)
		return(-1);
	int ret = render_node->effect->update(render_node);
	return(ret);
}

int daemon_update_render_nodes(struct razer_daemon *daemon)
{
	if(daemon->is_render_nodes_dirty)
		daemon_compute_render_nodes(daemon);
		//printf("daemon render_nodes to update:%d\n",daemon->render_nodes_num);
	int ret = 0;
	struct razer_fx_render_node *rn = NULL;
	for(int i = daemon->render_nodes_num-1;i>=0;i--)
	{
		rn = daemon->render_nodes[i];
		ret = daemon_update_render_node(daemon,rn);
		if(!ret || daemon_has_render_node_reached_render_limit(daemon,rn))
		{

			if(rn->next)
			{
				//exchange this render_node with the next one
				daemon->render_nodes[i] = rn->next;
				if(rn->next->output_frame_linked_uid == -1)
					daemon_free_rgb_frame(&rn->next->output_frame);
				if(rn->next->input_frame_linked_uid == -1)
					daemon_free_rgb_frame(&rn->next->input_frame);
				if(rn->next->second_input_frame_linked_uid == -1)
					daemon_free_rgb_frame(&rn->next->second_input_frame);
				rn->next->input_frame = rn->input_frame;
				rn->next->second_input_frame = rn->second_input_frame;
				rn->next->output_frame = rn->output_frame;
				rn->next->input_frame_linked_uid = rn->input_frame_linked_uid;
				rn->next->second_input_frame_linked_uid = rn->second_input_frame_linked_uid;
				//if(rn->output_frame_linked_uid!= -1)
				rn->next->output_frame_linked_uid = rn->output_frame_linked_uid;
				rn->next->prev = rn;
				rn->next->start_ticks = 0;
				rn->start_ticks = 0;
				rn->running = 0;

			}
		}
	}
	if(!rn->next && (!ret || daemon_has_render_node_reached_render_limit(daemon,rn)))
	{
		//root render_node effect returned 0
		//start next render_node in chain or default

	}
		//razer_clear_frame(daemon->render_node->input_frame);
		//daemon_update_render_node(daemon->render_node);
	razer_update_frame(daemon->chroma,daemon->frame_buffer);
	return(1);
}

int daemon_key_event_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node,int keycode,int pressed)
{
	if(!render_node || !render_node->effect)
		return(-1);

	if(!render_node->start_ticks)
	{
		render_node->start_ticks = razer_get_ticks();
		render_node->running  = 1;
	}
	if(!render_node->running)
		return(0);
	if(render_node->subs_num)
	{
		for(int i=0;i<render_node->subs_num;i++)
		{
			struct razer_fx_render_node *sub = render_node->subs[i];
			if(!sub->start_ticks)
			{
				sub->start_ticks = razer_get_ticks();
				sub->running  = 1;
			}
			if(!sub->running)
				continue;
			int sub_ret = daemon_key_event_render_node(daemon,sub,keycode,pressed);
			if(!sub_ret || daemon_has_render_node_reached_render_limit(daemon,sub) || !sub->running)
			{
				if(sub->next)
				{
					render_node->subs[i] = sub->next;
					sub->next->parent = render_node;
					sub->next->start_ticks = 0; 
				}
				sub->running = 0;
				//return(0);
			}
		}
	}
	if(!render_node->effect->key_event)
		return(-1);
	int ret = render_node->effect->key_event(render_node,keycode,pressed);
	return(ret);
}

int daemon_key_event_render_nodes(struct razer_daemon *daemon,int keycode,int pressed)
{
	if(daemon->is_render_nodes_dirty)
		daemon_compute_render_nodes(daemon);
	int ret = 0;
	struct razer_fx_render_node *rn = NULL;
	for(int i = daemon->render_nodes_num-1;i>=0;i--)
	{
		rn = daemon->render_nodes[i];
		ret = daemon_key_event_render_node(daemon,rn,keycode,pressed);
		if(!ret || daemon_has_render_node_reached_render_limit(daemon,rn))
		{
			if(rn->next)
			{
				//exchange this render_node with the next one
				daemon->render_nodes[i] = rn->next;
				if(rn->next->output_frame_linked_uid == -1)
					daemon_free_rgb_frame(&rn->next->output_frame);
				if(rn->next->input_frame_linked_uid == -1)
					daemon_free_rgb_frame(&rn->next->input_frame);
				if(rn->next->second_input_frame_linked_uid == -1)
					daemon_free_rgb_frame(&rn->next->second_input_frame);
				rn->next->input_frame = rn->input_frame;
				rn->next->second_input_frame = rn->second_input_frame;
				rn->next->output_frame = rn->output_frame;
				rn->next->input_frame_linked_uid = rn->input_frame_linked_uid;
				rn->next->second_input_frame_linked_uid = rn->second_input_frame_linked_uid;
				//if(rn->output_frame_linked_uid!= -1)
				rn->next->output_frame_linked_uid = rn->output_frame_linked_uid;
				rn->next->prev = rn;
				rn->next->start_ticks = 0;
				rn->start_ticks = 0;
				rn->running = 0;

			}
		}
	}
	if(!rn->next && (!ret || daemon_has_render_node_reached_render_limit(daemon,rn)))
	{
		//root render_node effect returned 0
		//start next render_node in chain or default
	}
	return(1);
}

int daemon_key_event_handler(struct razer_chroma *chroma,int keycode,int pressed)
{
	#ifdef USE_VERBOSE_DEBUGGING
		printf("daemon key event handler called: %d,%d\n",keycode,pressed);
	#endif
	daemon_key_event_render_nodes((struct razer_daemon*)chroma->tag,keycode,pressed);		
	return(1);
}

int daemon_run(struct razer_daemon *daemon)
{
    while(daemon->running)
	{	
		unsigned long ticks = razer_get_ticks();
		daemon_update_render_nodes(daemon);		
		#ifdef USE_DBUS
			daemon_dbus_handle_messages(daemon);
		#endif
		razer_update(daemon->chroma);
		razer_frame_limiter(daemon->chroma,daemon->render_node->effect->fps);
		unsigned long end_ticks = razer_get_ticks();
		#ifdef USE_DEBUGGING
			//printf("\rframe time:%ums,actual fps:%f (Wanted:%d)",end_ticks-ticks,1000.0f/(float)(end_ticks-ticks),daemon->render_node->effect->fps);
			printf("                                                                             \rft:%ums,fps:%f(%d)",end_ticks-ticks,1000.0f/(float)(end_ticks-ticks),daemon->render_node->effect->fps);
		#endif
	}
	return(1);
}

void daemon_queue_append_render_node(struct razer_queue *queue,struct razer_fx_render_node *render_node)
{
	if(queue->items!=NULL)
		queue->items = (void**)realloc(queue->items,sizeof(void*)*(queue->num+1));
	else
		queue->items = (void**)malloc(sizeof(void*));
	queue->items[queue->num] = render_node;
	queue->num++;
}

void daemon_compute_append_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(daemon->render_nodes!=NULL)
		daemon->render_nodes = (struct razer_fx_render_node**)realloc(daemon->render_nodes,sizeof(void*)*(daemon->render_nodes_num+1));
	else
		daemon->render_nodes = (struct razer_fx_render_node**)malloc(sizeof(void*));
	daemon->render_nodes[daemon->render_nodes_num] = render_node;
	daemon->render_nodes_num++;
}

void daemon_compute_append_queue(struct razer_daemon *daemon,struct razer_queue *queue)
{
	while(queue->pos<queue->num)
	{
		struct razer_fx_render_node *render_node = (struct razer_fx_render_node*)queue->items[queue->pos];
		daemon_compute_append_render_node(daemon,render_node);
		if(render_node->input_frame_linked_uid!=-1 && render_node->input_frame_linked_uid != 0)
			daemon_queue_append_render_node(queue,daemon_get_render_node(daemon,render_node->input_frame_linked_uid));
		if(render_node->second_input_frame_linked_uid!=-1 && render_node->second_input_frame_linked_uid != 0)
			daemon_queue_append_render_node(queue,daemon_get_render_node(daemon,render_node->second_input_frame_linked_uid));
		queue->pos++;
	}
}

void daemon_compute_render_nodes(struct razer_daemon *daemon)
{
	struct razer_queue *queue = daemon_create_queue();
	if(daemon->render_nodes)
	{
		free(daemon->render_nodes);
		daemon->render_nodes = NULL;
	}
	daemon->render_nodes_num = 0;
	struct razer_fx_render_node *rn = daemon_get_render_node(daemon,daemon->frame_buffer_linked_uid);
	if(rn)
	{
		daemon_queue_append_render_node(queue,rn);
		daemon_compute_append_queue(daemon,queue);
	}
	daemon_free_queue(&queue);
	daemon->is_render_nodes_dirty = 0;
}

struct razer_queue *daemon_create_queue(void)
{
	struct razer_queue *queue = (struct razer_queue*)malloc(sizeof(struct razer_queue));
	queue->items = NULL;
	queue->num = 0;
	queue->pos = 0;
	return(queue);
}

void daemon_free_queue(struct razer_queue **queue)
{
	if((*queue)->items)
		free((*queue)->items);
	free((*queue));
	(*queue) = NULL;
}

int daemon_register_effect(struct razer_daemon *daemon,struct razer_effect *effect)
{
	if(daemon->effects!=NULL)
		daemon->effects = (struct razer_effect**)realloc(daemon->effects,sizeof(struct razer_effect*)*(daemon->effects_num+1));
	else
		daemon->effects = (struct razer_effect**)malloc(sizeof(struct razer_effect*));
	daemon->effects[daemon->effects_num] = effect;
	daemon->effects[daemon->effects_num]->id = daemon->effects_uid++;
	daemon->effects_num++;
	return(daemon->effects[daemon->effects_num-1]->id);
}

int daemon_register_lib(struct razer_daemon *daemon,void *lib)
{
	if(daemon->libs!=NULL)
		daemon->libs = (void**)realloc(daemon->libs,sizeof(void*)*(daemon->libs_num+1));
	else
		daemon->libs = (void**)malloc(sizeof(void*));
	daemon->libs[daemon->libs_num] = lib;
	daemon->libs_num++;
	return(daemon->libs_num-1);
}


int daemon_register_render_node(struct razer_daemon *daemon,struct razer_fx_render_node *render_node)
{
	if(daemon->fx_render_nodes!=NULL)
		daemon->fx_render_nodes = (struct razer_fx_render_node**)realloc(daemon->fx_render_nodes,sizeof(struct razer_fx_render_node*)*(daemon->fx_render_nodes_num+1));
	else
		daemon->fx_render_nodes = (struct razer_fx_render_node**)malloc(sizeof(struct razer_fx_render_node*));
	daemon->fx_render_nodes[daemon->fx_render_nodes_num] = render_node;
	daemon->fx_render_nodes[daemon->fx_render_nodes_num]->id = daemon->fx_render_nodes_uid++;
	daemon->fx_render_nodes_num++;
	return(daemon->fx_render_nodes[daemon->fx_render_nodes_num-1]->id);
}
/*
int daemon_unregister_effect(struct razer_daemon *daemon,struct razer_effect *effect)
{
	//remove effect
	//fill gap if there is one left by effect
	//realloc
}
*/

//struct razer_fx_render_node *daemon_create_render_node(struct razer_daemon *daemon,struct razer_effect *effect,struct razer_rgb_frame *input_frame,struct razer_rgb_frame *second_input_frame,struct razer_rgb_frame *output_frame,char *description)
struct razer_fx_render_node *daemon_create_render_node(struct razer_daemon *daemon,struct razer_effect *effect,int input_render_node_uid,int second_input_render_node_uid,int output_render_node_uid,char *name,char *description)
{
	struct razer_fx_render_node *render_node = (struct razer_fx_render_node*)malloc(sizeof(struct razer_fx_render_node));
	render_node->daemon = daemon;
	//render_node->effect = effect;
	if(effect)
		render_node->effect = daemon_create_effect_instance(daemon,effect);
	else
		render_node->effect = NULL;
	render_node->opacity = 1.0f;
	if(input_render_node_uid == -1)
	{
		struct razer_rgb_frame *iframe = daemon_create_rgb_frame();
		render_node->input_frame = iframe;
		render_node->input_frame_linked_uid = -1;
	}
	else if(input_render_node_uid == 0) //set input to daemon output buffer
	{
		render_node->input_frame = daemon->frame_buffer;
		render_node->input_frame_linked_uid = 0;
	}
	else
	{
		struct razer_fx_render_node *rn = daemon_get_render_node(daemon,input_render_node_uid);
		render_node->input_frame = rn->output_frame;
		render_node->input_frame_linked_uid = input_render_node_uid;
	}

	if(second_input_render_node_uid == -1)
	{
		struct razer_rgb_frame *siframe = daemon_create_rgb_frame();
		render_node->second_input_frame = siframe;
		render_node->second_input_frame_linked_uid = -1;
	}
	else if(second_input_render_node_uid == 0) //set input to daemon output buffer
	{
		render_node->second_input_frame = daemon->frame_buffer;
		render_node->second_input_frame_linked_uid = 0;
	}
	else
	{
		struct razer_fx_render_node *srn = daemon_get_render_node(daemon,second_input_render_node_uid);
		render_node->second_input_frame = srn->output_frame;
		render_node->second_input_frame_linked_uid = second_input_render_node_uid;
	}

	if(output_render_node_uid == -1)
	{
		struct razer_rgb_frame *oframe = daemon_create_rgb_frame();
		render_node->output_frame = oframe;
		render_node->output_frame_linked_uid = -1;
	}
	else if(output_render_node_uid == 0) //set input to daemon output buffer
	{
		render_node->output_frame = daemon->frame_buffer;
		render_node->output_frame_linked_uid = 0;
	}
	/*else //not used
	{
		struct razer_fx_render_node *orn = daemon_get_render_node(daemon,output_render_node_uid);
		render_node->output_frame = orn->output_frame;
		render_node->output_frame_linked_uid = output_render_node_uid;
	}*/

	render_node->description = str_Copy(description);
	render_node->name = str_Copy(name);
	//render_node->fps = daemon->fps;
	render_node->compose_mode = RAZER_COMPOSE_MODE_MIX;
	render_node->next = NULL;
	render_node->parent = NULL;
	//render_node->parameters = NULL;
	//render_node->parameters_num = 0;
	render_node->subs = NULL;
	render_node->subs_num = 0;
	render_node->start_ticks = 0;
	render_node->running = 0;//set to 1 with first update
	render_node->limit_render_time_ms = 0;
	//render_node->continue_chain=1;
	render_node->loop_count = -1;
	return(render_node);
}


struct razer_effect *daemon_get_effect(struct razer_daemon *daemon,int uid)
{
	for(int i = 0;i<daemon->effects_num;i++)
	{
		if(daemon->effects[i]->id == uid)
			return(daemon->effects[i]);
	}
	return(NULL);
}

struct razer_fx_render_node *daemon_get_render_node(struct razer_daemon *daemon,int uid)
{
	for(int i = 0;i<daemon->fx_render_nodes_num;i++)
	{
		if(daemon->fx_render_nodes[i]->id == uid)
			return(daemon->fx_render_nodes[i]);
	}
	return(NULL);
}


struct razer_effect *daemon_create_effect(void)
{
	struct razer_effect *effect = (struct razer_effect*)malloc(sizeof(struct razer_effect));
	effect->parameters = (struct razer_parameters*)malloc(sizeof(struct razer_parameters));
	effect->parameters->items = NULL;
	effect->parameters->num = 0;
	effect->parameters->items_uid = 1;
	effect->name = NULL;
	effect->description = NULL;
	effect->open = NULL;
	effect->close = NULL;
	effect->update = NULL;
	effect->key_event = NULL;
	effect->dbus_event = NULL;
	effect->fps = 1;
	effect->input_usage_mask = 0;
	effect->class = 0;
	return(effect);
}

void daemon_free_effect(struct razer_effect **effect)
{
	if((*effect)->parameters)
		free((*effect)->parameters);
	free((*effect));
	(*effect) = NULL;
}

struct razer_parameter *daemon_create_parameter(void)
{
	struct razer_parameter *parameter = (struct razer_parameter*)malloc(sizeof(struct razer_parameter));
	return(parameter);
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
		case RAZER_PARAMETER_TYPE_RENDER_NODE:
			daemon_set_parameter_render_node(copy,daemon_get_parameter_render_node(parameter));
			break;
	}
	
	return(copy);
}

void daemon_free_parameter(struct razer_parameter **parameter)
{
	free((*parameter));
	(*parameter) = NULL;
}

void daemon_free_parameters(struct razer_parameters **parameters)
{
	if((*parameters)->num)
	{
		for(int i=0;i<(*parameters)->num;i++)
			daemon_free_parameter(&(*parameters)->items[i]);
			free((*parameters)->items);
	}
	free((*parameters));
	(*parameters) = NULL;
}


struct razer_rgb_frame *daemon_create_rgb_frame(void)
{
	struct razer_rgb_frame *frame = (struct razer_rgb_frame*)malloc(sizeof(struct razer_rgb_frame));
	razer_init_frame(frame);
	return(frame);
}

void daemon_free_rgb_frame(struct razer_rgb_frame **frame)
{
	free((*frame));
	(*frame) = NULL;
}

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

struct razer_parameter *daemon_create_parameter_int(char *key,char *description,int value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_INT;
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

struct razer_parameter *daemon_create_parameter_render_node(char *key,char *description,struct razer_fx_render_node *value)
{
	struct razer_parameter *parameter = daemon_create_parameter();
	parameter->key = key;
	parameter->description = description;
	parameter->value = (unsigned long)value;
	parameter->type = RAZER_PARAMETER_TYPE_RENDER_NODE;
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

void daemon_set_parameter_int(struct razer_parameter *parameter,int value)
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

int daemon_get_parameter_int(struct razer_parameter *parameter)
{
	if(!parameter)
		return(0);
	return((int)parameter->value);
}

struct razer_rgb *daemon_get_parameter_rgb(struct razer_parameter *parameter)
{
	if(!parameter)
		return(NULL);
	return((struct razer_rgb*)parameter->value);
}

struct razer_fx_render_node *daemon_get_parameter_render_node(struct razer_parameter *parameter)
{
	return((struct razer_fx_render_node*)parameter->value);
}


int daemon_add_parameter(struct razer_parameters *parameters,struct razer_parameter *parameter)
{
	if(parameters->items!=NULL)
		parameters->items = (struct razer_parameter**)realloc(parameters->items,sizeof(struct razer_parameter*)*(parameters->num+1));
	else
		parameters->items = (struct razer_parameter**)malloc(sizeof(struct razer_parameter*));
	parameters->items[parameters->num] = parameter;
	parameters->items[parameters->num]->id = parameters->items_uid++;
	parameters->num++;
	return(parameters->items[parameters->num-1]->id);
}

struct razer_parameter *daemon_get_parameter_by_index(struct razer_parameters *parameters,int index)
{
	if(parameters)
	{
		if(index < parameters->num)
			return(parameters->items[index]);
	}	
	return(NULL);
}



void daemon_render_node_add_sub(struct razer_fx_render_node *render_node,struct razer_fx_render_node *sub_node)
{
	if(render_node->subs!=NULL)
		render_node->subs = (struct razer_fx_render_node**)realloc(render_node->subs,sizeof(struct razer_fx_render_node*)*(render_node->subs_num+1));
	else
		render_node->subs = (struct razer_fx_render_node**)malloc(sizeof(struct razer_fx_render_node*));
	render_node->subs[render_node->subs_num] = sub_node;
	sub_node->parent = render_node;
	render_node->subs_num++;
}






/*

void sdl_update()
{
		SDL_Event event;
	    while(SDL_PollEvent(&event)) 
    	{
		    if(event.type == SDL_KEYUP)
    		{
		    	if(event.key.keysym.sym == SDLK_ESCAPE)
			    	done=1;
	      	}
		    if(event.type == SDL_MOUSEBUTTONUP)
    		{
		    	//if(event.key.keysym.sym == SDLK_ESCAPE)
				int w,h;
				SDL_GetWindowSize(window,&w,&h);
				int kw=w/22;
				int kh=(h-32)/6;
		    	if(event.button.y<h-32)
		    	{
		    		int kx = (event.button.x)/kw;
		    		int ky = event.button.y/kh;
		    		//printf("button pressed in:%d,%d\n",kx,ky);
					struct razer_rgb cr = {.r=128,.g=0,.b=0};
					set_key(keys,kx,ky,&cr);
		    	}
			    //	done=1;

	      	}
		    if(event.type == SDL_QUIT)
    		{
		    	done=1;
	      	}

 		}
		update_sdl(keys,renderer,window,tex);
}

*/

/*
void create_sdl_window()
{
   	SDL_Init(SDL_INIT_VIDEO);      
	SDL_Window *sdl_window;
	SDL_Renderer *sdl_renderer;
	SDL_CreateWindowAndRenderer(22*32, 6*32, SDL_WINDOW_RESIZABLE, &sdl_window, &sdl_renderer);
	SDL_SetWindowTitle(sdl_window,"Razer Chroma Setup/Debug");
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(sdl_renderer);
	SDL_RenderPresent(sdl_renderer);
	SDL_Texture *sdl_texture = SDL_CreateTexture(sdl_renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,22,6);
	load_icons(sdl_renderer,"icons",sdl_icons);
}


void close_sdl_window()
{
  	SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
*/

//list of last keystrokes
//time since hit /hitstamps


int daemonize()
{
	pid_t pid = 0;
	pid_t sid = 0;
	pid = fork();
	if(pid<0)
	{
		printf("razer_bcd: fork failed\n");
		exit(1);
	}
	if(pid)
	{
		#ifdef USE_DEBUGGING
			printf("killing razer_bcd parent process\n");
		#endif
		exit(0);
	}
	umask(0);
	sid = setsid();
	if(sid < 0)
	{
		printf("razer_bcd: setsid failed\n");
		exit(1);
	}
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	return(1);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	printf("Starting razer blackwidow chroma daemon\n");
	#ifndef USE_DEBUGGING
		daemonize();
	#endif

	struct razer_daemon *daemon=NULL;
	if(!(daemon=daemon_open()))
	{
		printf("razer_bcd: error initializing daemon\n");
		return(1);
	}
	daemon_run(daemon);
    daemon_close(&daemon);
}

#pragma GCC diagnostic pop


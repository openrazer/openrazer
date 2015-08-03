#include "razer_daemon_controller.h"


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

//end of dbus ifdef
#endif 

void dc_quit(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
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
	DBusMessageIter args;
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
	DBusMessageIter args;
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

int dc_render_node_create(struct razer_daemon_controller *controller,int effect_uid,char *name,char *description)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.render_node","create");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	dbus_message_iter_init_append(msg,&args);
	if(!dbus_message_iter_append_basic(&args,DBUS_TYPE_INT32,&effect_uid))
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

void dc_render_node_set(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.render_node","set");
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
}

/*int dc_render_node_parameter_get(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid)
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

void dc_render_node_parameter_set(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid)
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
*/

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

void dc_frame_buffer_connect(struct razer_daemon_controller *controller,int render_node_uid)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.frame_buffer","connect");
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
}

void dc_frame_buffer_disconnect(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.frame_buffer","disconnect");
	if(!msg)
		dc_error_close(controller,"Error creating Message\n");
	if(!dbus_connection_send_with_reply(controller->dbus,msg,&controller->pending,-1))
		dc_error_close(controller,"Out of memory!\n"); 
	if(!controller->pending)
		dc_error_close(controller,"No pending call\n"); 
	dbus_connection_flush(controller->dbus);
	dbus_message_unref(msg);
}

void dc_fx_list(struct razer_daemon_controller *controller)
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
	printf("fx List: %s\n",list);
	dbus_message_unref(msg);   
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

int dc_frame_buffer_get(struct razer_daemon_controller *controller)
{
	DBusMessage *msg;
	DBusMessageIter args;
	msg = dbus_message_new_method_call("org.voyagerproject.razer.daemon","/","org.voyagerproject.razer.daemon.frame_buffer","get");
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

struct razer_daemon_controller *dc_open(void)
{
	struct razer_daemon_controller *controller = (struct razer_daemon_controller*)malloc(sizeof(struct razer_daemon_controller));
	#ifdef USE_DBUS
	 	controller->dbus = NULL;
		#ifdef USE_DEBUGGING
			printf("dbus: opened\n");
		#endif
	 	if(!dc_dbus_open(controller))
	 	{
	 		free(controller);
			return(NULL);
		}
	#endif
 	return(controller);
}

void dc_close(struct razer_daemon_controller *controller)
{
	#ifdef USE_DBUS
		dc_dbus_close(controller);
	#endif
 	free(controller);
}

void dc_error_close(struct razer_daemon_controller *controller,char *message)
{
	printf("daemon controller error: %s\n");
	#ifdef USE_DBUS
		dc_dbus_close(controller);
	#endif
 	free(controller);
 	exit(1);
}

int dc_create_render_node(struct razer_daemon_controller *controller,int effect_uid,int input_render_node_uid,int second_input_render_node_uid,int output_render_node_uid,char *name,char *description)
{
}

struct razer_effect *dc_get_effect(struct razer_daemon_controller *controller,int effect_uid)
{
}

struct razer_fx_render_node *dc_get_render_node(struct razer_daemon_controller *controller,int render_node_uid)
{
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	printf("Executing razer blackwidow chroma daemon controller\n");

	struct razer_daemon_controller *controller=NULL;
	if(!(controller=dc_open()))
	{
		printf("razer_bcd_controller: error initializing daemon controller\n");
		return(1);
	}
	//dc_quit(controller);
	//dc_load_fx_lib(controller,"daemon/fx/pez2001_mixer_debug.so");
	//dc_fx_list(controller);
	dc_pause(controller);
	printf("daemon paused : %d\n",dc_is_paused(controller));
	dc_continue(controller);
	printf("daemon paused : %d\n",dc_is_paused(controller));
	dc_fps_set(controller,25);
	printf("fps:%d\n",dc_fps_get(controller));
    int uid = dc_frame_buffer_get(controller);
    printf("actual render_node uid: %d\n",uid);
    printf("parent uid: %d\n",dc_render_node_parent_get(controller,uid));
    dc_render_node_opacity_set(controller,uid,0.5f);
    printf("opacity: %f\n",dc_render_node_opacity_get(controller,uid));
    int cuid = dc_render_node_create(controller,6,"Copper","test");
    dc_frame_buffer_connect(controller,cuid);
    printf("set frame buffer to: %d\n",cuid);
    //dc_render_node_limit_render_time_ms_set(controller,uid,1000);

    dc_close(controller);
}

#pragma GCC diagnostic pop


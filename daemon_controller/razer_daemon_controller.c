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

int dc_render_node_parameter_get(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid)
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


/*void dc_render_node_parameter_set(struct razer_daemon_controller *controller,int render_node_uid,int parameter_uid)
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

const char *dc_helpmsg = "Usage: %s [OPTIONS]... [COMMAND] [PARAMETERS]...\n\
Send commands to razer_bcd daemon.\n\
\n\
Commands:\n\
  -q    Close daemon\n\
  -c    Continue rendering\n\
  -p    Pause rendering\n\
  -C    Create rendering node\n\
           1. Parameter: Effect uid - sets effect render node uses\n\
           2. Parameter: Name - sets the render nodes name\n\
           3. Parameter: Description - sets the render nodes description\n\
\n\
           For example: %s -C 1 \"My render node\" \"My description\"\n\
  -l    Load fx library\n\
           1. Parameter: Library filename\n\
  -f    Set fps rate\n\
           1. Parameter: fps rate\n\
  -g    Get fps rate\n\
           Returns: actual fps rate of daemon\n\
  -o    Set render node opacity\n\
           1. Parameter: render node uid - render node to set the opacity to\n\
           2. Parameter: opacity value - float value between 0.0 and 1.0\n\
  -O    Get render node opacity\n\
           1. Parameter: render node uid - render node to get the opacity of\n\
           Returns: opacity of render node as a float value between 0.0 and 1.0\n\
  -i    Is rendering paused ?\n\
           Returns: 0/1 running/paused\n\
  -x    Get fx list\n\
           Returns: fx list as json string\n\
  -a    Get the actual render node uid connected to the framebuffer\n\
           Returns: uid of node\n\
  -t    Get the parent of a render node\n\
           1. Parameter: render node uid - render node to get the parent of\n\
           Returns: uid of parent node\n\
  -L    Set render node rendering time limit\n\
           1. Parameter: render node uid - render node to set the time limit\n\
           2. Parameter: time limit value - time span in ms\n\
  -b    Connect frame buffer to render node\n\
           1. Parameter: render node uid - render node that gets connected to the frame buffer\n\
  -s    Add Sub-node to render node\n\
           1. Parameter: render node uid - render node the sub node should be added to\n\
           2. Parameter: sub node uid - sub node that gets added\n\
  -r    Connect input node to render nodes first input slot\n\
           1. Parameter: render node uid - render node the input node should be connected to\n\
           2. Parameter: input node uid - input node to connect\n\
  -r    Connect input node to render nodes second input slot\n\
           1. Parameter: render node uid - render node the input node should be connected to\n\
           2. Parameter: input node uid - input node to connect\n\
  -w    Get the next node of a render node\n\
           1. Parameter: render node uid - render node to get the next node of\n\
           Returns: uid of next node\n\
  -y    Set the next node of a render node\n\
           1. Parameter: render node uid - render node to get the next node of\n\
           2. Parameter: next node uid - next node to run after render node finished\n\
  -M    Get the move_linkage value of a render node\n\
           1. Parameter: render node uid - render node to get the move_linkage value of\n\
           Returns: uid of next node\n\
  -G    Set the move_linkage value of a render node\n\
           1. Parameter: render node uid - render node to get the next node of\n\
           2. Parameter: move_linkage - 0/1 activate/deactivate moving of framebuffer\n\
                         linkage of a render node\n\
  -d    Disconnect frame buffer\n\
  -h    Display this help and exit\n\
\n\
Options:\n\
  -v    More verbose output\n\
\n\
	DBUS must be running on the system to communicate with daemon.\n\
\n\
      Report bugs to <pez2001@voyagerproject.de>.\n";

int verbose = 0;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int main(int argc,char *argv[])
{
	char c;
	struct razer_daemon_controller *controller=NULL;
	if(!(controller=dc_open()))
	{
		printf("razer_bcd_controller: error initializing daemon controller\n");
		return(1);
	}
	while((c=getopt(argc,argv,"hvVcpqlfoigatOLxbdsrnwyCMG")) != -1)
	{
		switch(c)
		{
			case 'M':
				{
					int render_node_uid = atoi(argv[optind++]);
					int move_linkage = atoi(argv[optind]);
					if(verbose)
						printf("sending set move_linkage to %d for render node: %d command to daemon.\n",move_linkage,render_node_uid);
					dc_render_node_next_move_frame_buffer_linkage_set(controller,render_node_uid,move_linkage);
				}
				break;
			case 'G':
				{
					int render_node_uid = atoi(argv[optind++]);
					if(verbose)
					{
						printf("sending get move_linkage of render node: %d.\n",render_node_uid);
						printf("move linkage of render node: %d.\n",dc_render_node_next_move_frame_buffer_linkage_get(controller,render_node_uid));
					}
					else
						printf("%d",dc_render_node_next_move_frame_buffer_linkage_get(controller,render_node_uid));
				}
				break;
			case 'C':
				{
					int fx_uid = atoi(argv[optind++]);
					char *node_name = argv[optind++];
					char *node_description = argv[optind++];
					if(verbose)
					{
						printf("sending create render node command to daemon.\n");
						printf("new render node uid: %d.\n",dc_render_node_create(controller,fx_uid,node_name,node_description));
					}
					else
						printf("%d",dc_render_node_create(controller,fx_uid,node_name,node_description));

				}
				break;
			case 'y':
				{
					int render_node_uid = atoi(argv[optind++]);
					int next_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending set next node : %d for render node: %d command to daemon.\n",next_node_uid,render_node_uid);
					dc_render_node_next_set(controller,render_node_uid,next_node_uid);
				}
				break;
			case 'w':
				{
					int render_node_uid = atoi(argv[optind++]);
					if(verbose)
					{
						printf("sending get next node of render node: %d.\n",render_node_uid);
						printf("next render node: %d.\n",dc_render_node_next_get(controller,render_node_uid));
					}
					else
						printf("%d",dc_render_node_next_get(controller,render_node_uid));
				}
				break;
			case 'r':
				{
					int render_node_uid = atoi(argv[optind++]);
					int input_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending connect node : %d to render nodes: %d first input command to daemon.\n",input_node_uid,render_node_uid);
					dc_render_node_input_connect(controller,render_node_uid,input_node_uid);
				}
				break;
			case 'n':
				{
					int render_node_uid = atoi(argv[optind++]);
					int input_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending connect node : %d to render nodes: %d second input command to daemon.\n",input_node_uid,render_node_uid);
					dc_render_node_second_input_connect(controller,render_node_uid,input_node_uid);
				}
				break;
			case 's':
				{
					int render_node_uid = atoi(argv[optind++]);
					int sub_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending add sub node: %d to render node: %d command to daemon.\n",sub_node_uid,render_node_uid);
					dc_render_node_sub_add(controller,render_node_uid,sub_node_uid);
				}
				break;
			case 'b':
				{
					int render_node_uid = atoi(argv[optind]);
					if(verbose)
						printf("sending connect frame buffer to render node: %d command to daemon.\n",render_node_uid);
					dc_frame_buffer_connect(controller,render_node_uid);
				}
				break;
			case 'd':
				{
					if(verbose)
						printf("sending disconnect frame buffer command to daemon.\n");
					dc_frame_buffer_disconnect(controller);
				}
				break;
			case 'x':
				{
					char *list = dc_fx_list(controller);
					if(verbose)
					{	
						printf("sending get effects list command to daemon.\n");
						printf("daemon fx list:\n%s.\n",list);
					}
					else
						printf("%s",list);
					free(list);
				}
				break;
			case 'L':
				{
					int render_node_uid = atoi(argv[optind++]);
					int time_limit_ms = atoi(argv[optind]);
					if(verbose)
						printf("sending render node limit render time command to daemon.\n");
					dc_render_node_limit_render_time_ms_set(controller,render_node_uid,time_limit_ms);
				}
				break;
			case 't':
				{
					int render_node_uid = atoi(argv[optind]);
					if(verbose)
					{
						printf("sending get parent render node command to daemon.\n");
						printf("render node parent:%d.\n",dc_render_node_parent_get(controller,render_node_uid));
					}
					else
						printf("%d",dc_render_node_parent_get(controller,render_node_uid));
				}
				break;
			case 'a':
				if(verbose)
				{
					printf("sending get framebuffer connected render node command to daemon.\n");
					printf("daemon is running node:%d.\n",dc_frame_buffer_get(controller));
				}
				else
					printf("%d",dc_frame_buffer_get(controller));
				break;
			case 'g':
				if(verbose)
				{
					printf("sending get fps command to daemon.\n");
					printf("daemon is running at %d fps.\n",dc_fps_get(controller));
				}
				else
					printf("%d",dc_fps_get(controller));
				break;
			case 'i':
				if(verbose)
				{
					printf("sending is paused command to daemon.\n");
					int paused = dc_is_paused(controller);
					if(paused)
						printf("daemon is paused.\n");
					else
						printf("daemon is running.\n");
				}
				else
					printf("%d",dc_is_paused(controller));
				break;
			case 'o':
				{
					int render_node_uid = atoi(argv[optind++]);
					double opacity = atof(argv[optind]);
					if(verbose)
						printf("sending set opacity for render node: %d command to daemon: %f.\n",render_node_uid,opacity);
					dc_render_node_opacity_set(controller,render_node_uid,opacity);
				}
				break;
			case 'O':
				{
					int render_node_uid = atoi(argv[optind]);
					if(verbose)
					{
						printf("sending get opacity for render node: %d command to daemon.\n",render_node_uid);
						printf("render node opacity is to: %f.\n",dc_render_node_opacity_get(controller,render_node_uid));
					}
					else
						printf("%f",dc_render_node_opacity_get(controller,render_node_uid));
				}
				break;
			case 'f':
				{
					int fps = atoi(argv[optind]);
					if(verbose)
						printf("sending set fps command to daemon: %d.\n",fps);
					dc_fps_set(controller,fps);
				}
				break;
			case 'l':
				if(verbose)
					printf("sending load fx library command to daemon: library to load: \"%s\".\n",argv[optind]);
				dc_load_fx_lib(controller,argv[optind]);
				break;
			case 'c':
				if(verbose)
					printf("sending continue command to daemon.\n");
				dc_continue(controller);
				break;
			case 'p':
				if(verbose)
					printf("sending pause command to daemon.\n");
				dc_pause(controller);
				break;
			case 'q':
				if(verbose)
					printf("sending quit command to daemon.\n");
				dc_quit(controller);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'h':
				printf("Razer blackwidow chroma daemon controller\n");
				printf(dc_helpmsg,argv[0],argv[0]);
				return(0);
			case 'V':
				printf("razer_bcd daemon controller %d.%d (build %d)\n",MAJOR_VERSION,MINOR_VERSION);
				return(0);
			case '?':
				if(optopt == 'c')
					printf("Option -%c requires an argument.\n",optopt);
				else if(isprint(optopt))
					printf("Unknown option `-%c'.\n",optopt);
				else
					printf("Unknown option character `\\x%x'.\n",optopt);
				return(1);
			default:
				abort();
		}
	}
	dc_close(controller);
	return(0);


}

#pragma GCC diagnostic pop


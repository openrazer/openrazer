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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

dbus_bool_t daemon_dbus_check_user_handler(DBusConnection *connection, unsigned long uid, void *data)
{
	#ifdef USE_DEBUGGING
		printf("dbus: authenticated user:%lu\n",uid);
	#endif
	/*no user id check performed*/
	return(TRUE);
}

#pragma GCC diagnostic pop

int daemon_dbus_open(struct razer_daemon *daemon)
{
	DBusError error;
	dbus_error_init(&error);
	daemon->dbus = dbus_bus_get(DBUS_BUS_SYSTEM,&error);
	if(!daemon_dbus_error_check("open",&error))
		return(0);
	if(!daemon->dbus)
		return(0);
	dbus_connection_set_unix_user_function(daemon->dbus,daemon_dbus_check_user_handler,NULL,NULL);
	dbus_connection_set_allow_anonymous(daemon->dbus,TRUE);
	dbus_connection_set_route_peer_messages(daemon->dbus,TRUE);
	if(!daemon_dbus_error_check("open_user_check",&error))
		return(0);
	return(1);
}

void daemon_dbus_close(struct razer_daemon *daemon)
{
	if(daemon->dbus)
		dbus_connection_unref(daemon->dbus);
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
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node","reset"))
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
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage","set"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.sub","add"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.subs","list"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parameters","list"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parameter","set"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parameter","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.render_node.parameter.value","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.frame_buffer","connect"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.frame_buffer","disconnect"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","pause"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","is_paused"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","continue"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.frame_buffer","get"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","quit"))
		return(0);

	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","enable_macro_keys"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","raw_keyboard_brightness"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","set_game_mode"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","serial"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon","device_name"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.driver_effect","none"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.driver_effect","static"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.driver_effect","breath"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.driver_effect","reactive"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.driver_effect","wave"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.driver_effect","spectrum"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.driver_effect","starlight"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.devices","get_number_of_devices"))
		return(0);
	if(!daemon_dbus_add_method(daemon,"org.voyagerproject.razer.daemon.devices","set_active"))
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

	if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "enable_macro_keys"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method enable_macro_keys called\n");
		#endif

		/*char *device_path = str_CreateEmpty();
		device_path = str_CatFree(device_path, daemon->chroma->device_path);
		device_path = str_CatFree(device_path, "/macro_keys");

		#ifdef USE_DEBUGGING
			printf("Device path: %s\n", device_path);
		#endif

		write_to_device_file(device_path, "1", 1);
		free(device_path);
		*/
		razer_enable_macro_keys(daemon->chroma);
		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.devices", "set_active"))
	{
		int active=0;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&active);
			}
			dbus_message_iter_init_append(reply,&parameters);
			#ifdef USE_DEBUGGING
				printf("\ndbus: setting active device to: %d\n", active);
			#endif

			razer_set_active_device_id(daemon->chroma,active);

		}
		dbus_uint32_t serial = 0;
		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.devices", "get_number_of_devices"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get_number_of_devices called\n");
		#endif
		long num_devices = razer_get_num_devices(daemon->chroma);

		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);

		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&num_devices)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "serial"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method serial called\n");
		#endif
		char* serial_str = (char*)calloc(16, sizeof(char));
		razer_get_serial(daemon->chroma, &serial_str[0]);

		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);

		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&serial_str)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(serial_str);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "device_name"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method device_name called\n");
		#endif
		char* name_str = (char*)calloc(64, sizeof(char));
		razer_get_name(daemon->chroma, &name_str[0]);
		printf("\n\n\n\nDevice Name: %s\n\n\n\n", name_str);

		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);

		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&name_str))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_uint32_t serial = 0;
		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(name_str);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "raw_keyboard_brightness"))
	{
		int brightness=0;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&brightness);
			}
			dbus_message_iter_init_append(reply,&parameters);
			#ifdef USE_DEBUGGING
				printf("\ndbus: setting brightness to: %d\n", brightness);
			#endif

			/*char *device_path = str_CreateEmpty();
			device_path = str_CatFree(device_path, daemon->chroma->device_path);
			device_path = str_CatFree(device_path, "/set_brightness");

			#ifdef USE_DEBUGGING
				printf("Device path: %s\n", device_path);
			#endif

			char buf[32];
			sprintf(buf, "%d", brightness);
			write_to_device_file(device_path, buf, strlen(buf));
			free(device_path);
			*/
			razer_set_brightness(daemon->chroma,brightness);

		}
		dbus_uint32_t serial = 0;
		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "set_game_mode"))
	{
		int enable=0;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&enable);
			}
			dbus_message_iter_init_append(reply,&parameters);
			#ifdef USE_DEBUGGING
				printf("\ndbus: setting game mode to: %d\n", enable);
			#endif

			/*char *device_path = str_CreateEmpty();
			device_path = str_CatFree(device_path, daemon->chroma->device_path);
			device_path = str_CatFree(device_path, "/mode_game");

			#ifdef USE_DEBUGGING
				printf("Device path: %s\n", device_path);
			#endif

			char buf[32];
			sprintf(buf, "%d", enable);
			write_to_device_file(device_path, buf, strlen(buf));
			free(device_path);
			*/
			razer_set_game_mode(daemon->chroma,enable);

		}
		dbus_uint32_t serial = 0;
		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "custom"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method driver_effect.custom called\n");
		#endif
		/*	
		char *device_path = str_CreateEmpty();
		device_path = str_CatFree(device_path, daemon->chroma->device_path);
		device_path = str_CatFree(device_path, "/mode_custom");

		#ifdef USE_DEBUGGING
			printf("Device path: %s\n", device_path);
		#endif

		write_to_device_file(device_path, "1", 1);
		free(device_path);
		*/
		daemon->is_paused = 1;
		razer_set_custom_mode(daemon->chroma);

		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}	
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "set_key_row"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set_key_row called\n");
		#endif
				
		char *dbus_array; // Shouldn't have to free this as DBUS "should" do it
		int num_elements;
		/*
		char *device_path = str_CreateEmpty();
		device_path = str_CatFree(device_path, daemon->chroma->device_path);
		device_path = str_CatFree(device_path, "/set_key_row");

		#ifdef USE_DEBUGGING
			printf("Device path: %s\n", device_path);
		#endif
		*/

		reply = dbus_message_new_method_return(msg);
		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_ARRAY)
			{
				int element_type = dbus_message_iter_get_element_type (&parameters);
				if(element_type == DBUS_TYPE_BYTE) 
				{
					DBusMessageIter iter_sub;
					dbus_message_iter_recurse(&parameters, &iter_sub);
					dbus_message_iter_get_fixed_array (&iter_sub, &dbus_array, &num_elements);
					#ifdef USE_DEBUGGING
						printf("\ndbus-array: Number of elements %d\n", num_elements);
						printf("\ndbus-array: Elements 1,2,3,4 %u,%u,%u,%u\n", dbus_array[0] & 0xFF, dbus_array[1] & 0xFF, dbus_array[2] & 0xFF, dbus_array[3] & 0xFF);
					#endif
				}
			}
			dbus_message_iter_init_append(reply,&parameters);
			if(num_elements % 67 == 0 || num_elements % 46 == 0)// 67 Is standard packet size which the driver accepts. 1 BYTE row id, 22 RBG (3xBYTE) [firefly takes 46 bytes including index]
			{
				daemon->is_paused = 1;
			    //write_to_device_file(device_path, dbus_array, num_elements);
			    razer_set_led_row_buffered(daemon->chroma,(unsigned char*)dbus_array,num_elements);
		    }
			//free(device_path);
		}
		dbus_uint32_t serial = 0;
		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "none"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method enable_macro_keys called\n");
		#endif

		/*char *device_path = str_CreateEmpty();
		device_path = str_CatFree(device_path, daemon->chroma->device_path);
		device_path = str_CatFree(device_path, "/mode_none");

		#ifdef USE_DEBUGGING
			printf("Device path: %s\n", device_path);
		#endif

		write_to_device_file(device_path, "1", 1);
		free(device_path);
		*/
		razer_set_none_mode(daemon->chroma);
		daemon->is_paused = 1;


		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "static"))
	{
		struct razer_rgb color;
		/*unsigned char red=0;
		unsigned char green=0;
		unsigned char blue=0;
		*/
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&color.r);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&color.g);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&color.b);
			}
			dbus_message_iter_init_append(reply,&parameters);

			#ifdef USE_DEBUGGING
				printf("\ndbus: method set mode static called\n");
			#endif

			/*char *device_path = str_CreateEmpty();
			device_path = str_CatFree(device_path, daemon->chroma->device_path);
			device_path = str_CatFree(device_path, "/mode_static");

			#ifdef USE_DEBUGGING
				printf("Device path: %s -  R: %d, G: %d, B: %d\n", device_path, red, green, blue);
			#endif

			char buf[3] = {red, green, blue};
			write_to_device_file(device_path, buf, 3);
			free(device_path);
			*/
			razer_set_static_mode(daemon->chroma,&color);
			daemon->is_paused = 1;
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "breath"))
	{
		/*char *device_path = str_CreateEmpty();
		device_path = str_CatFree(device_path, daemon->chroma->device_path);
		device_path = str_CatFree(device_path, "/mode_breath");

		unsigned char red=0;
		unsigned char green=0;
		unsigned char blue=0;
		unsigned char red2=0;
		unsigned char green2=0;
		unsigned char blue2=0;
		unsigned char bytes=3;
		*/
		struct razer_rgb first_color;
		struct razer_rgb second_color;
		int colors = 0;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&first_color.r);
				colors++;
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&first_color.g);
				colors++;
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&first_color.b);
				colors++;
			}

			/*#ifdef USE_DEBUGGING
				printf("Device path: %s -  R: %d, G: %d, B: %d\n", device_path, red, green, blue);
			#endif
			*/

			//char buf[6] = {red, green, blue, 0, 0, 0};

			if(dbus_message_iter_has_next(&parameters)) // If breathing 2 colour mode
			{
				//bytes=6;
				dbus_message_iter_next(&parameters);
				if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
				{
					dbus_message_iter_get_basic(&parameters,&second_color.r);
					colors++;
				}
				dbus_message_iter_next(&parameters);
				if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
				{
					dbus_message_iter_get_basic(&parameters,&second_color.g);
					colors++;
				}
				dbus_message_iter_next(&parameters);
				if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
				{
					dbus_message_iter_get_basic(&parameters,&second_color.b);
					colors++;
				}

				/*#ifdef USE_DEBUGGING
					printf("Device path: %s -  R: %d, G: %d, B: %d, R2: %d, G2: %d, B2: %d \n", device_path, red, green, blue, red2, green2, blue2);
				#endif
				buf[3] = red2;
				buf[4] = green2;
				buf[5] = blue2;
				*/
			}

			dbus_message_iter_init_append(reply,&parameters);
			#ifdef USE_DEBUGGING
				printf("\ndbus: method set mode breath called\n");
			#endif
			daemon->is_paused = 1;
			//write_to_device_file(device_path, buf, bytes);
			switch(colors)
			{
				case 3:
					razer_set_one_color_breath_mode(daemon->chroma,&first_color);
					break;
				case 6:
					razer_set_breath_mode(daemon->chroma,&first_color,&second_color);
					break;
				default:
					razer_set_random_breath_mode(daemon->chroma);
					break;
			}
		} 
		else
		{
			/*char *device_path = str_CreateEmpty();
			device_path = str_CatFree(device_path, daemon->chroma->device_path);
			device_path = str_CatFree(device_path, "/mode_breath");

			#ifdef USE_DEBUGGING
				printf("Device path: %s -  Random breathing mode\n", device_path);
			#endif
			*/
			daemon->is_paused = 1;
			// Writing any bytes as long as its not 3 or 6 will trigger random breathing mode
			//write_to_device_file(device_path, "1", 1);
			razer_set_random_breath_mode(daemon->chroma);
		}

		//free(device_path);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "reactive"))
	{
		unsigned char speed=3;
		/*unsigned char red=255;
		unsigned char green=0;
		unsigned char blue=0;
		*/
		struct razer_rgb color;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&speed);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&color.r);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&color.g);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&color.b);
			}

			dbus_message_iter_init_append(reply,&parameters);

			#ifdef USE_DEBUGGING
				printf("\ndbus: method set mode reactive called\n");
			#endif

			/*char *device_path = str_CreateEmpty();
			device_path = str_CatFree(device_path, daemon->chroma->device_path);
			device_path = str_CatFree(device_path, "/mode_reactive");

			#ifdef USE_DEBUGGING
				printf("Device path: %s -  R: %d, G: %d, B: %d, Speed: %d\n", device_path, red, green, blue, speed);
			#endif
				
			char buf[4] = {speed, red, green, blue};
			write_to_device_file(device_path, buf, 4);
			free(device_path);
			*/
			daemon->is_paused = 1;
			razer_set_reactive_mode(daemon->chroma,speed,&color);

		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "wave"))
	{
		unsigned char direction=0;
		// None = 0
		// Right = 1
		// Left = 2
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_BYTE)
			{
				dbus_message_iter_get_basic(&parameters,&direction);
			}

			dbus_message_iter_init_append(reply,&parameters);

			#ifdef USE_DEBUGGING
				printf("\ndbus: method set mode wave called\n");
			#endif

			/*char *device_path = str_CreateEmpty();
			device_path = str_CatFree(device_path, daemon->chroma->device_path);
			device_path = str_CatFree(device_path, "/mode_wave");
			#ifdef USE_DEBUGGING
				if(direction == 0)
				{
					printf("Device path: %s -  Direction None\n", device_path);
				} 
				else if(direction == 1)
				{
					printf("Device path: %s -  Direction Right\n", device_path);
				} 
				else if(direction == 2)
				{
					printf("Device path: %s -  Direction Left\n", device_path);
				}
				else 
				{
					printf("Device path: %s -  Direction Unknown\n", device_path);
				}
			#endif
			char buf[32];
			sprintf(buf, "%d", direction);
			write_to_device_file(device_path, buf, strlen(buf));
			free(device_path);
			*/
			daemon->is_paused = 1;
			razer_set_wave_mode(daemon->chroma,direction);
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "spectrum"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method spectrum called\n");
		#endif

		/*char *device_path = str_CreateEmpty();
		device_path = str_CatFree(device_path, daemon->chroma->device_path);
		device_path = str_CatFree(device_path, "/mode_spectrum");

		#ifdef USE_DEBUGGING
			printf("Device path: %s\n", device_path);
		#endif

		write_to_device_file(device_path, "1", 1);
		free(device_path);
		*/
		daemon->is_paused = 1;
		razer_set_spectrum_mode(daemon->chroma);
		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.driver_effect", "starlight"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method starlight called\n");
		#endif

		daemon->is_paused = 1;
		razer_set_starlight_mode(daemon->chroma);
		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial))
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_nodes", "list")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list render_nodes called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *rn_list_json = str_CreateEmpty();
		rn_list_json = str_CatFree(rn_list_json,"{\n");
		rn_list_json = str_CatFree(rn_list_json," \"render_nodes_num\" : ");
		char *rn_num_string = str_FromLong(list_GetLen(daemon->fx_render_nodes));
		rn_list_json = str_CatFree(rn_list_json,rn_num_string);
		rn_list_json = str_CatFree(rn_list_json," ,\n");
		free(rn_num_string);
		rn_list_json = str_CatFree(rn_list_json," \"render_nodes_list\": [\n");
		for(int i=0;i<list_GetLen(daemon->fx_render_nodes);i++)
		{
			struct razer_fx_render_node *render_node = list_Get(daemon->fx_render_nodes,i);
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.devices", "list")) 
	{
		char **path = NULL;
		int device_uid = 0;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list devices called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path[0] != NULL)
			device_uid = atoi(path[0]);
		struct razer_chroma_device *device = daemon_get_device(daemon,device_uid);
		if(device)
		{
			struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
			char *rn_list_json = str_CreateEmpty();
			rn_list_json = str_CatFree(rn_list_json,"{\n");
			rn_list_json = str_CatFree(rn_list_json," \"render_nodes_num\" : ");
			char *rn_num_string = str_FromLong(list_GetLen(device_data->render_nodes));
			rn_list_json = str_CatFree(rn_list_json,rn_num_string);
			rn_list_json = str_CatFree(rn_list_json," ,\n");
			free(rn_num_string);
			rn_list_json = str_CatFree(rn_list_json," \"render_nodes_list\": [\n");
			for(int i=0;i<list_GetLen(device_data->render_nodes);i++)
			{
				struct razer_fx_render_node *render_node = list_Get(device_data->render_nodes,i);
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
		else
		{
	 		dbus_uint32_t serial = 0;
 			if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
				daemon_kill(daemon,"dbus: Out Of Memory!\n");
			dbus_connection_flush(daemon->dbus);
		}
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.device.render_nodes", "render_list")) 
	{
		char **path = NULL;
		int device_uid = 0;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list rendering render_nodes called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path[0] != NULL)
			device_uid = atoi(path[0]);
		struct razer_chroma_device *device = daemon_get_device(daemon,device_uid);
		if(device)
		{
			struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
			char *rn_list_json = str_CreateEmpty();
			rn_list_json = str_CatFree(rn_list_json,"{\n");
			rn_list_json = str_CatFree(rn_list_json," \"render_nodes_num\" : ");
			char *rn_num_string = str_FromLong(list_GetLen(device_data->render_nodes));
			rn_list_json = str_CatFree(rn_list_json,rn_num_string);
			rn_list_json = str_CatFree(rn_list_json," ,\n");
			free(rn_num_string);
			rn_list_json = str_CatFree(rn_list_json," \"render_nodes_list\": [\n");
			for(int i=0;i<list_GetLen(device_data->render_nodes);i++)
			{
				struct razer_fx_render_node *render_node = list_Get(device_data->render_nodes,i);
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
		else
		{
	 		dbus_uint32_t serial = 0;
 			if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
				daemon_kill(daemon,"dbus: Out Of Memory!\n");
			dbus_connection_flush(daemon->dbus);
		}
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parameters", "list")) 
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
				char *rn_json;
				if(i == render_node->effect->parameters->num - 1)
				{
					rn_json = daemon_parameter_to_json(render_node->effect->parameters->items[i], 1);
				} else {
					rn_json = daemon_parameter_to_json(render_node->effect->parameters->items[i], 0);
				}

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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parameter", "get"))
	{
		char **path = NULL;
		int rn_uid = -1;
		int p_index = -1;
		int a_index = -1;
		dbus_message_get_path_decomposed(msg,&path);
		int path_len = daemon_dbus_get_string_array_len(path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get render_nodes.parameter called for : %d\n",path_len);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path_len >= 2)
		{
			rn_uid = atoi(path[0]);
			p_index = atoi(path[1]);
			#ifdef USE_DEBUGGING
				printf("dbus: get parameter path : %d , %d\n",rn_uid,p_index);
				printf("dbus: get parameter max path render_node : %d\n",list_GetLen(daemon->fx_render_nodes));
			#endif
		}
		if(path_len == 3)
		{
			a_index = atoi(path[2]);
		}
			
		char *rn_list_json = str_CreateEmpty();
		dbus_free_string_array(path);
		if(list_GetLen(daemon->fx_render_nodes))
		{
			//struct razer_fx_render_node *render_node = daemon->fx_render_nodes[rn_uid];
			struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
			if(render_node && p_index>=0 && p_index < list_GetLen(render_node->effect->parameters))
			{
				struct razer_parameter *parameter = list_Get(render_node->effect->parameters,p_index);
				if(a_index != -1 && (parameter->type == RAZER_PARAMETER_TYPE_INT_ARRAY || parameter->type == RAZER_PARAMETER_TYPE_UINT_ARRAY || parameter->type == RAZER_PARAMETER_TYPE_FLOAT_ARRAY || parameter->type == RAZER_PARAMETER_TYPE_POS_ARRAY || parameter->type == RAZER_PARAMETER_TYPE_RGB_ARRAY))
				{
					char *parameter_json = daemon_parameter_array_to_json(parameter,a_index);
					rn_list_json = str_CatFree(rn_list_json,parameter_json);
					free(parameter_json);
				}
				else
				{
					char *parameter_json = daemon_parameter_to_json(parameter, 1);
					rn_list_json = str_CatFree(rn_list_json,parameter_json);
					free(parameter_json);
				}
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parameter", "set"))
	{
		char **path = NULL;
		int rn_uid = -1;
		int p_index = -1;
		int a_index = -1;
		dbus_message_get_path_decomposed(msg,&path);
		int path_len = daemon_dbus_get_string_array_len(path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set render_nodes.parameter called for : %d\n",path_len);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path_len >= 2)
		{
			rn_uid = atoi(path[0]);
			p_index = atoi(path[1]);
			#ifdef USE_DEBUGGING
				printf("dbus: set parameter path : %d , %d\n",rn_uid,p_index);
				printf("dbus: set parameter max path render_node : %d\n",list_GetLen(daemon->fx_render_nodes));
			#endif
		}
		if(path_len == 3)
		{
			a_index = atoi(path[2]);
		}
		dbus_free_string_array(path);
		if(dbus_message_iter_init(msg, &parameters) && list_GetLen(daemon->fx_render_nodes))
		{
			//struct razer_fx_render_node *render_node = daemon->fx_render_nodes[rn_uid];
			struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
			
			if(render_node && p_index>=0 && p_index < list_GetLen(render_node->effect->parameters))
			{
				struct razer_parameter *parameter = list_Get(render_node->effect->parameters,p_index);
				switch(parameter->type)
				{
					case RAZER_PARAMETER_TYPE_STRING:
						if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_STRING)
						{
							char *value;
							dbus_message_iter_get_basic(&parameters,&value);
							#ifdef USE_DEBUGGING
								printf("dbus: string parameter:%s\n",value);
							#endif
							daemon_set_parameter_string(parameter,str_Copy(value));
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
					case RAZER_PARAMETER_TYPE_UINT:
						if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_UINT32)
						{
							unsigned long value = 0;
							dbus_message_iter_get_basic(&parameters,&value);
							#ifdef USE_DEBUGGING
								printf("dbus: unsigned int parameter:%d\n",value);
							#endif
							daemon_set_parameter_uint(parameter,value);
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
					case RAZER_PARAMETER_TYPE_POS:
						{
							struct razer_pos *pos = daemon_get_parameter_pos(parameter);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: X parameter:%d\n",value);
								#endif
								pos->x = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: Y parameter:%d\n",value);
								#endif
								pos->y = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_RENDER_NODE:
						{
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: Node id parameter:%d\n",value);
								#endif
								struct razer_fx_render_node *dst_rn = daemon_get_render_node(daemon,(int)value);
								if(dst_rn)
									daemon_set_parameter_render_node(parameter,dst_rn);
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_FLOAT_RANGE:
						{
							struct razer_float_range *range = daemon_get_parameter_float_range(parameter);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_DOUBLE)
							{
								double value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min parameter:%f\n",value);
								#endif
								range->min = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_DOUBLE)
							{
								double value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max parameter:%f\n",value);
								#endif
								range->max = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_INT_RANGE:
						{
							struct razer_int_range *range = daemon_get_parameter_int_range(parameter);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min parameter:%l\n",value);
								#endif
								range->min = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max parameter:%l\n",value);
								#endif
								range->max = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_UINT_RANGE:
						{
							struct razer_uint_range *range = daemon_get_parameter_uint_range(parameter);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_UINT32)
							{
								unsigned long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min parameter:%ul\n",value);
								#endif
								range->min = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_UINT32)
							{
								unsigned long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max parameter:%l\n",value);
								#endif
								range->max = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_RGB_RANGE:
						{
							struct razer_rgb_range *range = daemon_get_parameter_rgb_range(parameter);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min R parameter:%d\n",value);
								#endif
								range->min->r = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min G parameter:%d\n",value);
								#endif
								range->min->g = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min B parameter:%d\n",value);
								#endif
								range->min->b = value;
							}
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max R parameter:%d\n",value);
								#endif
								range->max->r = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max G parameter:%d\n",value);
								#endif
								range->max->g = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max B parameter:%d\n",value);
								#endif
								range->max->b = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_POS_RANGE:
						{
							struct razer_pos_range *range = daemon_get_parameter_pos_range(parameter);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min X parameter:%d\n",value);
								#endif
								range->min->x = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: min Y parameter:%d\n",value);
								#endif
								range->min->y = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max X parameter:%d\n",value);
								#endif
								range->max->x = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: max Y parameter:%d\n",value);
								#endif
								range->max->y = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_FLOAT_ARRAY:
						{
							struct razer_float_array *array = daemon_get_parameter_float_array(parameter);
							if(a_index < 0 || a_index >= array->size)
								break;
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_DOUBLE)
							{
								double value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: float parameter:%f\n",value);
								#endif
								array->values[a_index] = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_INT_ARRAY:						
						{
							struct razer_int_array *array = daemon_get_parameter_int_array(parameter);
							if(a_index < 0 || a_index >= array->size)
								break;
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: integer parameter:%d\n",value);
								#endif
								array->values[a_index] = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_UINT_ARRAY:
						{
							struct razer_uint_array *array = daemon_get_parameter_uint_array(parameter);
							if(a_index < 0 || a_index >= array->size)
								break;
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_UINT32)
							{
								unsigned long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: integer parameter:%u\n",value);
								#endif
								array->values[a_index] = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_POS_ARRAY:
						{
							struct razer_pos_array *array = daemon_get_parameter_pos_array(parameter);
							if(a_index < 0 || a_index >= array->size)
								break;
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: integer parameter:%d\n",value);
								#endif
								array->values[a_index]->x = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: integer parameter:%d\n",value);
								#endif
								array->values[a_index]->y = value;
							}
						}
						break;
					case RAZER_PARAMETER_TYPE_RGB_ARRAY:
						{
							struct razer_rgb_array *array = daemon_get_parameter_rgb_array(parameter);
							if(a_index < 0 || a_index >= array->size)
								break;
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: integer parameter:%d\n",value);
								#endif
								array->values[a_index]->r = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: integer parameter:%d\n",value);
								#endif
								array->values[a_index]->g = value;
							}
							dbus_message_iter_next(&parameters);
							if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
							{
								long value = 0;
								dbus_message_iter_get_basic(&parameters,&value);
								#ifdef USE_DEBUGGING
									printf("dbus: integer parameter:%d\n",value);
								#endif
								array->values[a_index]->b = value;
							}
						}
						break;
				}
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node", "create"))
	{
		int fx_uid=0;
		int device_uid = 0;
		char *name = NULL;
		char *description = NULL;
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&fx_uid);
			}
			dbus_message_iter_next(&parameters);
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&device_uid);
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

			if(device_uid && fx_uid && description && name)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_chroma_device *device = daemon_get_device(daemon,device_uid);
				if(device)
				{
					struct razer_fx_render_node *rn = daemon_create_render_node(device,daemon_get_effect(daemon,fx_uid),-1,-1,-1,name,description);
					daemon_register_render_node(daemon,rn);
					#ifdef USE_DEBUGGING
						printf("\ndbus: created render_node:%d\n",rn->id);
					#endif
					/*char *rn_uid_json = str_CreateEmpty();
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
					*/
					if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&rn->id)) 
						daemon_kill(daemon,"dbus: Out Of Memory!\n");
				}
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node", "reset")) 
	{
		char **path = NULL;
		int rn_uid = -1;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method reset render_node called for : %s\n",path[0]);
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		struct razer_fx_render_node *render_node = daemon_get_render_node(daemon,rn_uid);
		if(render_node)
		{
			//daemon_reset_render_node(daemon,render_node);
			daemon_reset_render_node(render_node);
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.device.default_render_node", "set"))
	{
		char **path = NULL;
		int device_uid = 0;
		int rn_uid=0;
		dbus_message_get_path_decomposed(msg,&path);
		if(path[0] != NULL)
			device_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);
		struct razer_chroma_device *device = daemon_get_device(daemon,device_uid);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&rn_uid);
			}
			if(rn_uid && device)
			{
				struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				#ifdef USE_DEBUGGING
					printf("\ndbus: setting default render_node to: %d (@%x)\n",rn_uid,rn);
					//fflush(stdout);
				#endif
				if(rn)
					device_data->default_render_node = rn;
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.opacity", "set"))
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.opacity", "get"))
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.input", "connect"))
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
				daemon_connect_input(rn,dst_rn);
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.second_input", "connect"))
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
				daemon_connect_second_input(rn,dst_rn);
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.next", "set"))
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.next", "get"))
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
	else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage", "set"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method set render_node.next.move_frame_buffer_linkage called for : %s\n",path[0]);
		#endif
		int rn_uid=0;
		int move_linkage=0;
		if(path[0] != NULL)
			rn_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&move_linkage);
			}
			if(rn_uid && (move_linkage == 1 || move_linkage == 0))
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				rn->move_frame_buffer_linkage_to_next = move_linkage;
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage", "get"))
	{
		char **path = NULL;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get render_node.next.move_frame_buffer_linkage called for : %s\n",path[0]);
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
			if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&rn->move_frame_buffer_linkage_to_next)) 
				daemon_kill(daemon,"dbus: Out Of Memory!\n");
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.parent", "get"))
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
		int parent_uid = 0;
		if(rn_uid)
		{
			dbus_message_iter_init_append(reply,&parameters);
			struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
			if(rn->parent)
				parent_uid = rn->parent->id;
		}
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&parent_uid)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");

 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.limit_render_time_ms", "set"))
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.subs", "list")) 
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
			char *rn_num_string = str_FromLong(list_GetLen(render_node->subs));
			rn_list_json = str_CatFree(rn_list_json,rn_num_string);
			free(rn_num_string);
			if(list_GetLen(render_node->subs))
			{
				rn_list_json = str_CatFree(rn_list_json," ,\n");
				rn_list_json = str_CatFree(rn_list_json," \"subs_list\": [\n");
				for(int i=0;i<list_GetLen(render_node->subs);i++)
				{
					char *rn_json = daemon_render_node_to_json(list_Get(render_node->subs,i));
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.render_node.sub", "add"))
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
		if(dbus_message_iter_init(msg, &parameters) && list_GetLen(daemon->fx_render_nodes))
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.device.frame_buffer", "connect"))
	{
		char **path = NULL;
		int device_uid = 0;
		int rn_uid=0;
		dbus_message_get_path_decomposed(msg,&path);
		if(path[0] != NULL)
			device_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);

		struct razer_chroma_device *device = daemon_get_device(daemon,device_uid);
		//struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;

		if(dbus_message_iter_init(msg, &parameters))
		{
			if(dbus_message_iter_get_arg_type(&parameters) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&parameters,&rn_uid);
			}
			if(rn_uid && device)
			{
				dbus_message_iter_init_append(reply,&parameters);
				struct razer_fx_render_node *rn = daemon_get_render_node(daemon,rn_uid);
				#ifdef USE_DEBUGGING
					printf("\ndbus: connecting output frame buffer to render_node to: %d (@%x)\n",rn_uid,rn);
				#endif
				daemon_connect_frame_buffer(device,rn);
			}
		}
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.device.frame_buffer", "disconnect"))
	{
		char **path = NULL;
		int device_uid = 0;
		dbus_message_get_path_decomposed(msg,&path);
		if(path[0] != NULL)
			device_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);
		struct razer_chroma_device *device = daemon_get_device(daemon,device_uid);
		//struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
		if(device)
			daemon_disconnect_frame_buffer(device);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.device.frame_buffer", "get")) 
	{
		char **path = NULL;
		int device_uid = 0;
		dbus_message_get_path_decomposed(msg,&path);
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get frame_buffer called\n");
		#endif
		if(path[0] != NULL)
			device_uid = atoi(path[0]);
		dbus_free_string_array(path);
		reply = dbus_message_new_method_return(msg);
		struct razer_chroma_device *device = daemon_get_device(daemon,device_uid);
		if(device)
		{
			struct razer_daemon_device_data *device_data = (struct razer_daemon_device_data*)device->tag;
			dbus_message_iter_init_append(reply,&parameters);
			/*char *rn_uid_json = str_CreateEmpty();
			rn_uid_json = str_CatFree(rn_uid_json,"{\n");
			rn_uid_json = str_CatFree(rn_uid_json," \"uid\" : ");
			char *rn_uid_string = str_FromLong(daemon->frame_buffer_linked_uid);
			rn_uid_json = str_CatFree(rn_uid_json,rn_uid_string);
			rn_uid_json = str_CatFree(rn_uid_json,"\n");
			free(rn_uid_string);
			rn_uid_json = str_CatFree(rn_uid_json,"}\n");
			if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&rn_uid_json)) 
				daemon_kill(daemon,"dbus: Out Of Memory!\n");
			free(rn_uid_json);*/
			if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&device_data->frame_buffer_linked_uid)) 
				daemon_kill(daemon,"dbus: Out Of Memory!\n");
		}
		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "pause")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method pause called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		daemon->is_paused = 1;
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "continue")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method continue called\n");
		#endif
		daemon->is_paused = 0;
		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fps", "set")) 
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
			dbus_message_iter_get_basic(&parameters,&daemon->fps);
			#ifdef USE_DEBUGGING
				printf("parameter:%d\n",daemon->fps);
			#endif
		}
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&daemon->fps)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fps", "get")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method get fps called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&daemon->fps)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "is_paused")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method is_paused called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_INT32,&daemon->is_paused)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fx", "list")) 
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method list fx called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *fx_list_json = str_CreateEmpty();
		fx_list_json = str_CatFree(fx_list_json,"{\n");
		fx_list_json = str_CatFree(fx_list_json," \"effects_num\" : ");
		char *effects_num_string = str_FromLong(list_GetLen(daemon->effects));
		fx_list_json = str_CatFree(fx_list_json,effects_num_string);
		fx_list_json = str_CatFree(fx_list_json," ,\n");
		free(effects_num_string);
		fx_list_json = str_CatFree(fx_list_json," \"effects_list\": [\n");
		int list_length = list_GetLen(daemon->effects);
		for(int i=0;i<list_length;i++)
		{
			struct razer_effect *effect = list_Get(daemon->effects,i);
			char *effect_json;
			if(i == list_length - 1)
			{
				effect_json = daemon_effect_to_json(effect, 1);
			} else {
				effect_json = daemon_effect_to_json(effect, 0);
			}
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon.fx.lib", "load"))
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
    else if(dbus_message_is_method_call(msg, "org.voyagerproject.razer.daemon", "quit"))
	{
		#ifdef USE_DEBUGGING
			printf("\ndbus: method quit daemon called for\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		daemon->running = 0;
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


	/*
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
			razer_set_all(daemon->chroma->keys,&col);
			razer_update_keys(daemon->chroma,daemon->chroma->keys);
			//usleep((1000*1000)/255);
		}
		reply = dbus_message_new_method_return(msg);
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
	}
	*/
    else if(dbus_message_is_method_call(msg, "org.freedesktop.DBus.Introspectable", "Introspect")) 
	{
		#ifdef USE_DEBUGGING
			printf("dbus: method Introspect called\n");
		#endif
		reply = dbus_message_new_method_return(msg);
		dbus_message_iter_init_append(reply,&parameters);
		char *xml_data_start = 
		"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n\
		<node>\n\
			<interface name=\"org.freedesktop.DBus.Introspectable\">\n\
				<method name=\"Introspect\">\n\
					<arg direction=\"out\" name=\"data\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon\">\n\
				<method name=\"quit\">\n\
				</method>\n\
				<method name=\"pause\">\n\
				</method>\n\
				<method name=\"is_paused\">\n\
					<arg direction=\"out\" name=\"is_paused\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"continue\">\n\
				</method>\n\
				<method name=\"enable_macro_keys\">\n\
				</method>\n\
				<method name=\"raw_keyboard_brightness\">\n\
					<arg direction=\"in\" name=\"brightness\" type=\"y\">\n\
					</arg>\n\
				</method>\n\
		        <method name=\"set_game_mode\">\n\
					<arg direction=\"in\" name=\"enable\" type=\"y\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"serial\">\n\
					<arg direction=\"out\" name=\"serial_number\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"device_name\">\n\
					<arg direction=\"out\" name=\"device_name\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.driver_effect\">\n\
				<method name=\"none\">\n\
				</method>\n\
				<method name=\"custom\">\n\
				</method>\n\
				<method name=\"spectrum\">\n\
				</method>\n\
				<method name=\"starlight\">\n\
				</method>\n\
				<method name=\"static\">\n\
					<arg direction=\"in\" name=\"red\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"green\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"blue\" type=\"y\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"set_key_row\">\n\
					<arg direction=\"in\" name=\"data\" type=\"ay\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"breath\">\n\
				</method>\n\
				<method name=\"breath\">\n\
					<arg direction=\"in\" name=\"red\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"green\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"blue\" type=\"y\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"breath\">\n\
					<arg direction=\"in\" name=\"red\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"green\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"blue\" type=\"y\">\n\
					</arg>\n\
				    <arg direction=\"in\" name=\"red2\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"green2\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"blue2\" type=\"y\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"reactive\">\n\
				    <arg direction=\"in\" name=\"speed\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"red\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"green\" type=\"y\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"blue\" type=\"y\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"wave\">\n\
					<arg direction=\"in\" name=\"direction\" type=\"y\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.fps\">\n\
				<method name=\"set\">\n\
					<arg direction=\"in\" name=\"fps_rate\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"get\">\n\
					<arg direction=\"out\" name=\"fps_rate\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.fx\">\n\
				<method name=\"list\">\n\
					<arg direction=\"out\" name=\"fx_list_json\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"load\">\n\
					<arg direction=\"in\" name=\"fx_so_pathname\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_nodes\">\n\
				<method name=\"list\">\n\
					<arg direction=\"out\" name=\"render_nodes_list_json\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.device.render_nodes\">\n\
				<method name=\"render_list\">\n\
					<arg direction=\"out\" name=\"render_list_render_nodes_list_json\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n"; //split done to remove warning
			char *xml_data_end = "<interface name=\"org.voyagerproject.razer.daemon.device.default_render_node\">\n\
				<method name=\"set\">\n\
					<arg direction=\"in\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node\">\n\
				<method name=\"create\">\n\
					<arg direction=\"in\" name=\"effect_uid\" type=\"i\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"name\" type=\"s\">\n\
					</arg>\n\
					<arg direction=\"in\" name=\"description\" type=\"s\">\n\
					</arg>\n\
					<arg direction=\"out\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.parameters\">\n\
				<method name=\"list\">\n\
					<arg direction=\"out\" name=\"parameters_list_json\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.parameter\">\n\
				<method name=\"set\">\n\
					<arg direction=\"in\" name=\"parameter_value\" type=\"v\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"get\">\n\
					<arg direction=\"out\" name=\"parameter_value\" type=\"v\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.opacity\">\n\
				<method name=\"set\">\n\
					<arg direction=\"in\" name=\"opacity\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"get\">\n\
					<arg direction=\"out\" name=\"opacity\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.input\">\n\
				<method name=\"connect\">\n\
					<arg direction=\"in\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.second_input\">\n\
				<method name=\"connect\">\n\
					<arg direction=\"in\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.next\">\n\
				<method name=\"set\">\n\
					<arg direction=\"in\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"get\">\n\
					<arg direction=\"out\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.next.move_frame_buffer_linkage\">\n\
				<method name=\"set\">\n\
					<arg direction=\"in\" name=\"move_linkage_bool\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"get\">\n\
					<arg direction=\"out\" name=\"move_linkage_bool\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.parent\">\n\
				<method name=\"get\">\n\
					<arg direction=\"out\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.limit_render_time_ms\">\n\
				<method name=\"set\">\n\
					<arg direction=\"in\" name=\"limit_render_time_ms\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.subs\">\n\
				<method name=\"list\">\n\
					<arg direction=\"out\" name=\"render_node_subs_list_json\" type=\"s\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.render_node.sub\">\n\
				<method name=\"add\">\n\
					<arg direction=\"in\" name=\"compute_render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.device.frame_buffer\">\n\
				<method name=\"connect\">\n\
					<arg direction=\"in\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"disconnect\">\n\
				</method>\n\
				<method name=\"get\">\n\
					<arg direction=\"out\" name=\"render_node_uid\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
			<interface name=\"org.voyagerproject.razer.daemon.devices\">\n\
				<method name=\"get_number_of_devices\">\n\
					<arg direction=\"out\" name=\"num_devices\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
				<method name=\"set_active\">\n\
					<arg direction=\"in\" name=\"device_index\" type=\"i\">\n\
					</arg>\n\
				</method>\n\
			</interface>\n\
		</node>\n";
/*			<interface name=\"org.freedesktop.DBus.Properties\">\n\
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
*/
		char *xml_data = str_CreateEmpty();
		xml_data = str_CatFree(xml_data,xml_data_start);
		xml_data = str_CatFree(xml_data,xml_data_end);
		if(!dbus_message_iter_append_basic(&parameters,DBUS_TYPE_STRING,&xml_data)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
 		dbus_uint32_t serial = 0;
 		if(!dbus_connection_send(daemon->dbus,reply,&serial)) 
			daemon_kill(daemon,"dbus: Out Of Memory!\n");
		dbus_connection_flush(daemon->dbus);
		free(xml_data);
	}

	/*
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
	*/
	dbus_message_unref(msg);
	return(1);
}

//end of dbus ifdef
#endif 


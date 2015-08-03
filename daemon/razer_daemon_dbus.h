#ifndef _RAZER_DAEMON_DBUS_H_
#define _RAZER_DAEMON_DBUS_H_


#ifdef USE_DBUS
	#include <dbus/dbus.h>

	int daemon_dbus_open(struct razer_daemon *daemon);
	void daemon_dbus_close(struct razer_daemon *daemon);
	int daemon_dbus_handle_messages(struct razer_daemon *daemon);


	int daemon_dbus_error_check(char*message,DBusError *error);
	dbus_bool_t daemon_dbus_check_user_handler(DBusConnection *connection, unsigned long uid, void *data);
	int daemon_dbus_add_method(struct razer_daemon *daemon,char *interface_name, char *method_name);
	int daemon_dbus_announce(struct razer_daemon *daemon);
	int daemon_dbus_get_string_array_len(char **path);
#endif

#endif
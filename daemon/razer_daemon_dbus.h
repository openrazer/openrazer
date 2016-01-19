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
#ifndef _RAZER_DAEMON_CONTROLLER_H_
#define _RAZER_DAEMON_CONTROLLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include "getopt.h"
#include "ctype.h"


#include "../lib/razer_chroma.h"
#include "../daemon/razer_daemon.h"

#ifdef USE_DBUS
	#include <dbus/dbus.h>
#endif


struct razer_daemon_controller 
{
	int running;
	#ifdef USE_DBUS
		DBusConnection *dbus;
		DBusPendingCall *pending;
	#endif
};

#endif
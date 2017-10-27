#!/usr/bin/env python3
#
# This program monitors the screensaver DBus interfaces and toggles the
# respective methods on the openrazer daemon


import logging
import sys
import argparse

from gi.repository import GObject, GLib, Gio
from enum import Enum

DBUS_SCREENSAVER_INTERFACES = ('org.freedesktop.ScreenSaver',
                               'org.gnome.ScreenSaver',
                               'org.mate.ScreenSaver')


class ScreensaverState(Enum):
    SCREENSAVER_OFF = 0
    SCREENSAVER_ON = 1


class ScreensaverMonitorDBusError(Exception):
    pass


class ScreensaverMonitor(object):
    # The openrazer bus type
    BUS_TYPE = Gio.BusType.SYSTEM

    def __init__(self, verbose=False):
        if verbose:
            logging.basicConfig(level=logging.DEBUG)
        else:
            logging.basicConfig(level=logging.INFO)

        self._logger = logging.getLogger('razer.screensaver')
        self._logger.info("Initialising DBus Screensaver Monitor")

        # DBus proxy to org.razer.devices
        self._proxy = None
        self._init_subscriptions()
        self._watch_openrazer()

    def _watch_openrazer(self):
        """
        Set up a watch for the openrazer daemon DBus name.
        """
        try:
            self._watch_id = Gio.bus_watch_name(self.BUS_TYPE,
                                                "org.razer",
                                                Gio.BusNameWatcherFlags.AUTO_START,
                                                self._connect_openrazer,
                                                self._disconnect_openrazer)
        except GLib.Error as e:
            self._logger.critical("Failed to set up watch for openrazer-daemon")
            raise ScreensaverMonitorDBusError(e.message)

    def _connect_openrazer(self, connection, name, name_owner):
        """
        Connect to the openrazer daemon DBus name.
        """
        try:
            p = Gio.DBusProxy.new_for_bus_sync(self.BUS_TYPE,
                                               Gio.DBusProxyFlags.NONE, None,
                                               "org.razer",
                                               "/org/razer",
                                               "razer.daemon",
                                               None)
            version = p.call_sync("version", None, Gio.DBusCallFlags.NO_AUTO_START,
                                  500, None).unpack()[0]
            self._logger.info("Connected to openrazer-daemon v{}".format(version))

            self._proxy = Gio.DBusProxy.new_for_bus_sync(self.BUS_TYPE,
                                                         Gio.DBusProxyFlags.NONE,
                                                         None,
                                                         "org.razer",
                                                         "/org/razer",
                                                         "razer.devices",
                                                         None)
            # We throw away the result, this is just to test-connect so we
            # fail early.
            self._proxy.call_sync("getDevices", None,
                                  Gio.DBusCallFlags.NO_AUTO_START,
                                  500, None).unpack()[0]
        except GLib.Error as e:
            self._logger.critical("Failed to connect to openrazer-daemon")
            raise ScreensaverMonitorDBusError(e.message)

    def _disconnect_openrazer(self, connection, name):
        """
        Disconnect from the openrazer daemon DBus name.
        """
        self._logger.info("openrazer-daemon bus disappeared")
        self._proxy = None

    def _init_subscriptions(self):
        """
        Set up signal subscriptions to the various screensaver DBus
        interfaces on the session bus.
        """
        self._subscriptions = []
        self._session_bus = Gio.bus_get_sync(Gio.BusType.SESSION, None)
        # This should be synced from the interfaces
        self._state = ScreensaverState.SCREENSAVER_OFF

        for screensaver_interface in DBUS_SCREENSAVER_INTERFACES:
            self._logger.debug("Subscribing to '{}'".format(screensaver_interface))
            s = self._session_bus.signal_subscribe(None, screensaver_interface,
                                                   'ActiveChanged',
                                                   None, None, 0,
                                                   self._signal_callback,
                                                   None)
            self._subscriptions.append(s)

    def shutdown(self):
        for s in self._subscriptions:
            self._session_bus.signal_unsubscribe(s)
        Gio.bus_unwatch_name(self._watch_id)

    def _signal_callback(self, connection, name_owner, object_path, interface, signal, val, data):
        """
        Callback for DBus signal notifications
        """
        self._logger.debug("Received DBus signal {} {} {}".format(interface, signal, val))
        active = bool(val[0])
        self.set_screensaver_state(active)

    def set_screensaver_state(self, active):
        """
        Change the screensaver state to be active or inactive
        """
        if active:
            state = ScreensaverState.SCREENSAVER_ON
            if state == self._state:
                return
        else:
            state = ScreensaverState.SCREENSAVER_OFF
            if state == self._state:
                return

        self._set_device_state(active)

    def _set_device_state(self, state):
        if state == ScreensaverState.SCREENSAVER_ON:
            what = "Suspending"
            method = "suspendDevice"
        else:
            what = "Resuming"
            method = "resumeDevice"

        try:
            devices = self._proxy.call_sync("getDevices", None,
                                            Gio.DBusCallFlags.NO_AUTO_START,
                                            500, None).unpack()[0]
            for d in devices:
                self._logger.debug("{} device {}".format(what, d))
                p = Gio.DBusProxy.new_for_bus_sync(self.BUS_TYPE,
                                                   Gio.DBusProxyFlags.NONE,
                                                   None,
                                                   "org.razer",
                                                   "/org/razer/device/{}".format(d),
                                                   "razer.device.misc",
                                                   None)

                p.call_sync(method, None, Gio.DBusCallFlags.NO_AUTO_START, 500, None)

            self._state = state
        except GLib.Error as e:
            self._logger.critical("Failed to suspend/resume devices")
            raise ScreensaverMonitorDBusError(e.message)

    def monitor(self):
        """
        Monitor the screensaver interfaces and update our device state
        accordingly.
        """
        GObject.MainLoop().run()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='''
        This tool monitors the screensaver DBus interfaces for screensaver
        activation and toggles device suspend/resume on the openrazer-daemon
        as the screensaver toggles.''')
    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose logging', default=False)
    args = parser.parse_args()

    try:
        sm = ScreensaverMonitor(verbose=args.verbose)
        sm.monitor()
    except KeyboardInterrupt:
        sm.shutdown()
    except ScreensaverMonitorDBusError:
        sys.exit(1)

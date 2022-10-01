#!/usr/bin/python3
#
# References:
#    https://github.com/openrazer/openrazer/blob/master/daemon/openrazer_daemon/misc/screensaver_monitor.py
#    https://stackoverflow.com/questions/37320352/receive-dbus-signals-with-python
#
# [!] Make sure this key is set in ~/.config/openrazer/razer.conf:
#   devices_off_on_screensaver = False
#

import dbus
import openrazer.client
from gi.repository import GLib
from dbus.mainloop.glib import DBusGMainLoop

DBUS_SCREENSAVER_INTERFACES = (
    'org.cinnamon.ScreenSaver',
    'org.freedesktop.ScreenSaver',
    'org.gnome.ScreenSaver',
    'org.mate.ScreenSaver',
    'org.xfce.ScreenSaver',
)


def signal_callback(active):
    if active:
        print("Lock screen/screensaver activated")
        # Set to red
        device.fx.static(255, 0, 0)
    else:
        print("Lock screen/screensaver deactivated")
        # Set to green
        device.fx.static(0, 255, 0)


DBusGMainLoop(set_as_default=True)
bus = dbus.SessionBus()

# Register screensaver signals
for bus_name in DBUS_SCREENSAVER_INTERFACES:
    bus.add_signal_receiver(signal_callback, dbus_interface=bus_name, signal_name='ActiveChanged')

# Select the first device and sync effect to all devices
devman = openrazer.client.DeviceManager()
devman.sync_effects = True
device = devman.devices[0]

# Press CTRL+C to interrupt
loop = GLib.MainLoop()
loop.run()

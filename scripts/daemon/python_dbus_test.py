#!/usr/bin/env python3

import dbus


system_bus = dbus.SystemBus()
dbus_daemon_object = system_bus.get_object("org.voyagerproject.razer.daemon", "/")

# Provides:
# breath(byte red, byte green, byte blue)
# none()
# reactive(byte red, byte green, byte blue)
# spectrum()
# static(byte red, byte green, byte blue)
# wave(byte direction)
dbus_driver_effect_object = dbus.Interface(dbus_daemon_object, "org.voyagerproject.razer.daemon.driver_effect")

# Provides:
# enable_macro_keys()
# raw_keyboard_brightness(byte brightness)
# set_game_mode(byte enable)
dbus_daemon_controls = dbus.Interface(dbus_daemon_object, "org.voyagerproject.razer.daemon")

dbus_daemon_controls.pause()

payload = b'\x03\xFF\x00\x00'
for i in range(0, 21):
    payload += b'\x00\x00\x00'

#dbus_driver_effect_object.set_key_row(payload)

payload += b'\x02\xFF\x00\x00'
for i in range(0, 21):
    payload += b'\x00\x00\x00'

dbus_driver_effect_object.set_key_row(payload)

dbus_driver_effect_object.custom()
#!/usr/bin/env python3

import dbus

class DaemonInterface(object):
    def __init__(self):
      try:
        # Load up the DBUS
        system_bus = dbus.SystemBus()
        self.dbus_daemon_object = system_bus.get_object("org.voyagerproject.razer.daemon", "/")

        # Provides:
        # breath(byte red, byte green, byte blue)
        # none()
        # reactive(byte red, byte green, byte blue)
        # spectrum()
        # static(byte red, byte green, byte blue)
        # wave(byte direction)
        self.dbus_driver_effect_object = dbus.Interface(self.dbus_daemon_object, "org.voyagerproject.razer.daemon.driver_effect")

        # Provides:
        # enable_macro_keys()
        # raw_keyboard_brightness(byte brightness)
        # set_game_mode(byte enable)
        self.dbus_daemon_controls = dbus.Interface(self.dbus_daemon_object, "org.voyagerproject.razer.daemon")

      except:
        print("Failed to connect to the dbus service. Is the razer_bcd service running?")
        exit()

    def set_effect(self, effect_type, p1=1, p2=255, p3=255):
        print("[Daemon] Setting effect: \"{0}\"".format(effect_type))

        if effect_type == "breath":
            # Expects an RGB parameter
            print("[Daemon] Parameters: {0},{1},{2}".format(p1,p2,p3))
            self.dbus_driver_effect_object.breath(p1,p2,p3)

        elif effect_type == "none":
            # No parameters
            self.dbus_driver_effect_object.none()

        elif effect_type == "reactive":
            # Expects an RGB parameter
            print("[Daemon] Parameters: {0},{1},{2}".format(p1,p2,p3))
            self.dbus_driver_effect_object.reactive(p1,p2,p3)

        elif effect_type == "spectrum":
            # No parameters
            self.dbus_driver_effect_object.spectrum()

        elif effect_type == "static":
            # Expects an RGB parameter
            print("[Daemon] Parameters: {0},{1},{2}".format(p1,p2,p3))
            self.dbus_driver_effect_object.static(p1,p2,p3)

        elif effect_type == "wave":
            # Expects a integer parameter
            # 1 = Wave Right
            # 2 = Wave Left
            print("[Daemon] Parameters: "+str(p1))
            self.dbus_driver_effect_object.wave(p1)
        else:
            print("[Daemon] Invalid effect \"{0}\"".format(effect_type))

    def set_brightness(self, brightness):
        raw = brightness
        percent = round( (brightness / 255.0) * 100 )
        print("[Daemon] Brightness Set: {0} % ({1}/255)".format(percent, raw))
        self.dbus_daemon_controls.raw_keyboard_brightness(brightness)

    def marco_keys(self, state):
        if state:
            print("[Daemon] Marco Keys: Enabled")
            self.dbus_daemon_controls.enable_macro_keys()
        elif not state:
            print("[Daemon] Restart the 'razer_bcd' service to disable marco keys.")
        else:
            print("[Daemon] Invalid parameter for 'MarcoKeys' = {0}".format(state))

    def game_mode(self, state):
        if state:
            print("[Daemon] Game Mode: Enabled")
            self.dbus_daemon_controls.set_game_mode(1)
        elif not state:
            print("[Daemon] Game Mode: Disabled")
            self.dbus_daemon_controls.set_game_mode(0)
        else:
            print("[Daemon] Invalid parameter for 'GameMode' = {0}".format(state))

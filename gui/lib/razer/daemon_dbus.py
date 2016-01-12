#!/usr/bin/env python3

import dbus
import razer.keyboard

class DaemonInterface(object):
    """
    Interface to the daemon via DBUS
    """
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
        """
        Set effect on keyboard

        :param effect_type: Keyboard effect type
        :type effect_type: str

        :param p1: Parameter 1
        :type p1: int

        :param p2: Parameter 2
        :type p2: int

        :param p3: Parameter 3
        :type p3: int
        """
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
            print("[Daemon] Parameters: {0}".format(p1))
            self.dbus_driver_effect_object.wave(p1)
        else:
            print("[Daemon] Invalid effect \"{0}\"".format(effect_type))

    def set_brightness(self, brightness):
        """
        Set keyboard brightness

        :param brightness: Brightness value byte scaled (0-255)
        :type brightness: int
        """
        percent = round( (brightness / 255.0) * 100 )
        print("[Daemon] Brightness Set: {0} % ({1}/255)".format(percent, brightness))
        self.dbus_daemon_controls.raw_keyboard_brightness(brightness)

    def marco_keys(self, enable):
        """
        Enable macro keys on the keyboard

        :param enable: Boolean
        :type enable: bool
        """
        if enable:
            print("[Daemon] Marco Keys: Enabled")
            self.dbus_daemon_controls.enable_macro_keys()
        elif not enable:
            print("[Daemon] Restart the 'razer_bcd' service to disable marco keys.")
        else:
            print("[Daemon] Invalid parameter for 'MarcoKeys' = {0}".format(enable))

    def game_mode(self, enable):
        """
        Enable or disable game mode

        :param enable: Boolean
        :type enable: bool
        """
        if enable:
            print("[Daemon] Game Mode: Enabled")
            self.dbus_daemon_controls.set_game_mode(True)
        else:
            print("[Daemon] Game Mode: Disabled")
            self.dbus_daemon_controls.set_game_mode(False)

    def set_custom_colour(self, keyboard_object):
        """
        Set the colour of the keyboard to a custom colour

        :param keyboard_object: Keyboard object
        :type keyboard_object: razer.keyboard.KeyboardColour
        """
        assert type(keyboard_object) is razer.keyboard.KeyboardColour, "keyboard_object is not a KeyboardColour object"

        keyboard_payload = keyboard_object.get_total_binary()

        self.dbus_driver_effect_object.set_key_row(keyboard_payload)
        self.dbus_driver_effect_object.custom()
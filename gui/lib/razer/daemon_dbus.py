#!/usr/bin/env python3

import dbus
import sys

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

        # Output success message
        print('Successfully connected to dbus service.')

      except:
        print("Failed to connect to the dbus service. Is the razer_bcd service running?")
        sys.exit(1)


    def set_effect(self, effect_type, p1=None, p2=None, p3=None, p4=None, p5=None, p6=None):
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

        :param p4: Parameter 4
        :type p4: int

        :param p5: Parameter 5
        :type p5: int

        :param p6: Parameter 6
        :type p6: int
        """
        print("[DBUS] Set effect: \"{0}\"".format(effect_type))

        def validate_parameters(expected_no):
          # TODO: Validate all functions - set_brightness, game_mode, marco_keys, etc.
          validated = 0
          for parameter in p1, p2, p3, p4, p5, p6:
              if validated >= expected_no:
                  return True
              if parameter is None:
                  print("[DBUS] Invalid effect parameters. Expecting {0} but got {1}.".format(expected_no,validated))
                  return False
              else:
                  validated += 1

        if effect_type == "none":
            # No parameters
            self.dbus_driver_effect_object.none()

        elif effect_type == "breath":
            # Expects: <1 for random> or <red1> <green1> <blue1> [red2] [green2] [blue2]
            # Two colour parameters
            if p6 is not None:
                if validate_parameters(5):
                    print("[DBUS] Breath: Two colours with RGB: {0},{1},{2} and {3},{4},{5}".format(p1,p2,p3,p4,p5,p6))
                    self.dbus_driver_effect_object.breath(p1,p2,p3,p4,p5,p6, signature='yyyyyy')

            # One colour parameters
            if p2 is not None and p6 is None:
                if validate_parameters(3):
                    print("[DBUS] Breath: One colour with RGB: {0},{1},{2}".format(p1,p2,p3))
                    self.dbus_driver_effect_object.breath(p1,p2,p3, signature='yyy')

            # Random mode parameters
            if p1 == 1 and p2 is None:
                if validate_parameters(1):
                    print("[DBUS] Breath: Random Mode".format(p1))  # Random Mode
                    self.dbus_driver_effect_object.breath(signature='')
                    return

        elif effect_type == "reactive":
            # Expects <speed> <red> <green> <blue>
            # 1 = Fast
            # 2 = Normal
            # 3 = Slow
            if validate_parameters(4):
                print("[DBUS] Speed: {0}, RGB: {1},{2},{3}".format(p1,p2,p3,p4))
                self.dbus_driver_effect_object.reactive(p1,p2,p3,p4)

        elif effect_type == "spectrum":
            # No parameters
            self.dbus_driver_effect_object.spectrum()

        elif effect_type == "static":
            if validate_parameters(3):
                print("[DBUS] RGB: {0},{1},{2}".format(p1,p2,p3))
                # Expects <red> <green> <blue>
                self.dbus_driver_effect_object.static(p1,p2,p3)

        elif effect_type == "wave":
            # Expects <direction>
            # 0 = None
            # 1 = Wave Right
            # 2 = Wave Left
            if validate_parameters(1):
                print("[DBUS] Direction: {0}".format(p1))
                self.dbus_driver_effect_object.wave(p1)
        else:
            print("[DBUS] Invalid effect \"{0}\"".format(effect_type))

    def set_brightness(self, brightness):
        """
        Set keyboard brightness

        :param brightness: Brightness value byte scaled (0-255)
        :type brightness: int
        """
        percent = round( (brightness / 255.0) * 100 )
        print("[DBUS] Brightness Set: {0} % ({1}/255)".format(percent, brightness))
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
        else:
            print("[DBUS] Restart the 'razer_bcd' service to disable marco keys.")

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

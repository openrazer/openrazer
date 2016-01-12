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

        # Output success message
        print('Successfully connected to dbus service.')

      except:
        print("Failed to connect to the dbus service. Is the razer_bcd service running?")
        exit()


    def set_effect(self, effect_type, p1=None, p2=None, p3=None, p4=None, p5=None, p6=None):
        print("[DBUS] Set effect: \"{0}\"".format(effect_type))

        def validate_parameters(expected_no):
          # TODO: Validate all functions - set_brightness, game_mode, marco_keys, etc.
          validated = 0
          for parameter in p1, p2, p3, p4, p5, p6:
              if validated >= expected_no:
                  return True
              if parameter == None:
                  print("[DBUS] Invalid effect parameters. Expecting {0} but got {1}.".format(expected_no,validated))
                  return False
              else:
                  validated = validated + 1

        if effect_type == "none":
            # No parameters
            self.dbus_driver_effect_object.none()

        elif effect_type == "breath":
            # Expects: <1 for random> or <red1> <green1> <blue1> [red2] [green2] [blue2]
            if validate_parameters(1) == True:
                if p1 == '1' and p2 == 'None':
                    print("[DBUS] Breath: Random Mode".format(p1))  # Random Mode
                    self.dbus_driver_effect_object.breath(p1)
                    print('p1')
                elif p4 == None:
                    if validate_parameters(3) == True:
                        print("[DBUS] Breath: One colour with RGB: {0},{1},{2}".format(p1,p2,p3))  # One colour
                        self.dbus_driver_effect_object.breath(p1,p2,p3)
                    print('p4')
                else:
                    if validate_parameters(5) == True:
                        print("[DBUS] Breath: Two colours with RGB: {0},{1},{2} and {3},{4},{5}".format(p1,p2,p3,p4,p5,p6))  # Two colours
                        self.dbus_driver_effect_object.breath(p1,p2,p3,p4,p5,p6)

        elif effect_type == "reactive":
            # Expects <speed> <red> <green> <blue>
            # 1 = Fast
            # 2 = Normal
            # 3 = Slow
            if validate_parameters(4) == True:
                print("[DBUS] Speed: {0}, RGB: {1},{2},{3}".format(p1,p2,p3,p4))
                self.dbus_driver_effect_object.reactive(p1,p2,p3,p4)

        elif effect_type == "spectrum":
            # No parameters
            self.dbus_driver_effect_object.spectrum()

        elif effect_type == "static":
            if validate_parameters(3) == True:
                print("[DBUS] RGB: {0},{1},{2}".format(p1,p2,p3))
                # Expects <red> <green> <blue>
                self.dbus_driver_effect_object.static(p1,p2,p3)

        elif effect_type == "wave":
            # Expects <direction>
            # 0 = None
            # 1 = Wave Right
            # 2 = Wave Left
            if validate_parameters(1) == True:
                print("[DBUS] Direction: {0}".format(p1))
                self.dbus_driver_effect_object.wave(p1)
        else:
            print("[DBUS] Invalid effect \"{0}\"".format(effect_type))

    def set_brightness(self, brightness):
        raw = brightness
        percent = round( (brightness / 255.0) * 100 )
        print("[DBUS] Brightness Set: {0} % ({1}/255)".format(percent, raw))
        self.dbus_daemon_controls.raw_keyboard_brightness(brightness)

    def marco_keys(self, state):
        if state:
            print("[DBUS] Marco Keys: Enabled")
            self.dbus_daemon_controls.enable_macro_keys()
        elif not state:
            print("[DBUS] Restart the 'razer_bcd' service to disable marco keys.")
        else:
            print("[DBUS] Invalid parameter for 'MarcoKeys' = {0}".format(state))

    def game_mode(self, state):
        if state:
            print("[DBUS] Game Mode: Enabled")
            self.dbus_daemon_controls.set_game_mode(1)
        elif not state:
            print("[DBUS] Game Mode: Disabled")
            self.dbus_daemon_controls.set_game_mode(0)
        else:
            print("[DBUS] Invalid parameter for 'GameMode' = {0}".format(state))

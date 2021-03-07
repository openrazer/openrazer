"""
Contains functionality specific to keyboard-like devices
"""
import dbus as _dbus

from openrazer.client.constants import MACRO_LED_STATIC, MACRO_LED_BLINK
from openrazer.client.devices import RazerDevice as __RazerDevice, BaseDeviceFactory as __BaseDeviceFactory


class RazerKeyboard(__RazerDevice):
    def __init__(self, serial, vid_pid=None, daemon_dbus=None):
        super().__init__(serial, vid_pid=vid_pid, daemon_dbus=daemon_dbus)

        # Capabilities
        self._capabilities['game_mode_led'] = self._has_feature('razer.device.led.gamemode')
        self._capabilities['macro_mode_led'] = self._has_feature('razer.device.led.macromode', 'setMacroMode')
        self._capabilities['macro_mode_led_effect'] = self._has_feature('razer.device.led.macromode', 'setMacroEffect')
        self._capabilities['macro_mode_modifier'] = self._has_feature('razer.device.macro', 'setModeModifier')

        # Setup base stuff if need be
        if self.has('game_mode_led'):
            self._dbus_interfaces['game_mode_led'] = _dbus.Interface(self._dbus, "razer.device.led.gamemode")

        if self.has('macro_mode_led'):
            self._dbus_interfaces['macro_mode_led'] = _dbus.Interface(self._dbus, "razer.device.led.macromode")

        if self.has('lighting_profile_led_red') or self.has('lighting_profile_led_green') or self.has('lighting_profile_led_blue'):
            self._dbus_interfaces['profile_led'] = _dbus.Interface(self._dbus, "razer.device.lighting.profile_led")

    @property
    def game_mode_led(self) -> bool:
        """
        Get game mode LED state

        :return: LED state
        :rtype: bool
        """
        if self.has('game_mode_led'):
            return self._dbus_interfaces['game_mode_led'].getGameMode()
        else:
            return False

    @game_mode_led.setter
    def game_mode_led(self, value: bool):
        """
        Set game mode LED state

        :param value: LED State
        :type value: bool
        """
        if self.has('game_mode_led'):
            if value:
                self._dbus_interfaces['game_mode_led'].setGameMode(True)
            else:
                self._dbus_interfaces['game_mode_led'].setGameMode(False)

    @property
    def macro_mode_led(self) -> bool:
        """
        Get macro mode LED state

        :return: LED state
        :rtype: bool
        """
        if self.has('macro_mode_led'):
            return self._dbus_interfaces['macro_mode_led'].getMacroMode()
        else:
            return False

    @macro_mode_led.setter
    def macro_mode_led(self, value: bool):
        """
        Set macro mode LED state

        :param value: LED State
        :type value: bool
        """
        if self.has('macro_mode_led'):
            if value:
                self._dbus_interfaces['macro_mode_led'].setMacroMode(True)
            else:
                self._dbus_interfaces['macro_mode_led'].setMacroMode(False)

    @property
    def macro_mode_led_effect(self) -> int:
        """
        Get macro mode LED effect

        Can get either blinking or static
        :return: Effect ID
        :rtype: int
        """
        if self.has('macro_mode_led_effect'):
            return self._dbus_interfaces['macro_mode_led'].getMacroEffect()
        else:
            return False

    @macro_mode_led_effect.setter
    def macro_mode_led_effect(self, value: int):
        """
        Set macro mode LED effect

        Can set to either MACRO_LED_STATIC or MACRO_LED_BLINK
        :param value: Effect ID
        :type value: int
        """
        if self.has('macro_mode_led_effect') and value in (MACRO_LED_STATIC, MACRO_LED_BLINK):
            self._dbus_interfaces['macro_mode_led'].setMacroEffect(value)

    @property
    def profile_led_red(self) -> bool:
        """
         Get red profile LED state

         :return: Red profile LED state
         :rtype: bool
         """
        if self.has('lighting_profile_led_red'):
            return bool(self._dbus_interfaces['profile_led'].getRedLED())
        else:
            return False

    @profile_led_red.setter
    def profile_led_red(self, enable: bool):
        """
        Set red profile LED state

        :param enable: Status of red profile LED
        :type enable: bool
        """
        if self.has('lighting_profile_led_red'):
            self._dbus_interfaces['profile_led'].setRedLED(enable)

    @property
    def profile_led_green(self) -> bool:
        """
        Get green profile LED state

        :return: Green profile LED state
        :rtype: bool
        """
        if self.has('lighting_profile_led_green'):
            return bool(self._dbus_interfaces['profile_led'].getGreenLED())
        else:
            return False

    @profile_led_green.setter
    def profile_led_green(self, enable: bool):
        """
        Set green profile LED state

        :param enable: Status of green profile LED
        :type enable: bool
        """
        if self.has('lighting_profile_led_green'):
            self._dbus_interfaces['profile_led'].setGreenLED(enable)

    @property
    def profile_led_blue(self) -> bool:
        """
        Get blue profile LED state

        :return: Blue profile LED state
        :rtype: bool
        """
        if self.has('lighting_profile_led_blue'):
            return bool(self._dbus_interfaces['profile_led'].getBlueLED())
        else:
            return False

    @profile_led_blue.setter
    def profile_led_blue(self, enable: bool):
        """
        Set blue profile LED state

        :param enable: Status of blue profile LED
        :type enable: bool
        """
        if self.has('lighting_profile_led_blue'):
            self._dbus_interfaces['profile_led'].setBlueLED(enable)


DEVICE_PID_MAP = {

}


class RazerKeyboardFactory(__BaseDeviceFactory):
    @staticmethod
    def get_device(serial, vid_pid=None, daemon_dbus=None):
        if vid_pid is None:
            pid = 0xFFFF
        else:
            pid = vid_pid[1]

        device_class = DEVICE_PID_MAP.get(pid, RazerKeyboard)
        return device_class(serial, vid_pid=vid_pid, daemon_dbus=daemon_dbus)

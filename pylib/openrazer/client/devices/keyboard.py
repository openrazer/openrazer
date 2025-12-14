# SPDX-License-Identifier: GPL-2.0-or-later

"""
Contains functionality specific to keyboard-like devices
"""

from openrazer.client.constants import MACRO_LED_STATIC, MACRO_LED_BLINK
from openrazer.client.devices import RazerDevice as __RazerDevice, BaseDeviceFactory as __BaseDeviceFactory


class RazerKeyboard(__RazerDevice):
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
            raise NotImplementedError()

    @game_mode_led.setter
    def game_mode_led(self, value: bool) -> None:
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
        else:
            raise NotImplementedError()

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
            raise NotImplementedError()

    @macro_mode_led.setter
    def macro_mode_led(self, value: bool) -> None:
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
        else:
            raise NotImplementedError()

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
            raise NotImplementedError()

    @macro_mode_led_effect.setter
    def macro_mode_led_effect(self, value: int) -> None:
        """
        Set macro mode LED effect

        Can set to either MACRO_LED_STATIC or MACRO_LED_BLINK
        :param value: Effect ID
        :type value: int
        """
        if self.has('macro_mode_led_effect'):
            if value in (MACRO_LED_STATIC, MACRO_LED_BLINK):
                self._dbus_interfaces['macro_mode_led'].setMacroEffect(value)
            else:
                raise ValueError(f"Unexpected MACRO_LED_* value passed {value}")
        else:
            raise NotImplementedError()

    @property
    def keyswitch_optimization(self) -> bool:
        """
        Get keyswitch optimization state

        :return: Keyswitch optimization state
        :rtype: bool
        """
        if self.has('keyswitch_optimization'):
            return self._dbus_interfaces['keyswitch_optimization'].getKeyswitchOptimization()
        else:
            raise NotImplementedError()

    @keyswitch_optimization.setter
    def keyswitch_optimization(self, value: bool) -> None:
        """
        Set keyswitch optimization state

        :param value: Keyswitch optimization state
        :type value: bool
        """
        if self.has('keyswitch_optimization'):
            self._dbus_interfaces['keyswitch_optimization'].setKeyswitchOptimization(value)
        else:
            raise NotImplementedError()

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
            raise NotImplementedError()

    @profile_led_red.setter
    def profile_led_red(self, enable: bool) -> None:
        """
        Set red profile LED state

        :param enable: Status of red profile LED
        :type enable: bool
        """
        if self.has('lighting_profile_led_red'):
            self._dbus_interfaces['profile_led'].setRedLED(enable)
        else:
            raise NotImplementedError()

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
            raise NotImplementedError()

    @profile_led_green.setter
    def profile_led_green(self, enable: bool) -> None:
        """
        Set green profile LED state

        :param enable: Status of green profile LED
        :type enable: bool
        """
        if self.has('lighting_profile_led_green'):
            self._dbus_interfaces['profile_led'].setGreenLED(enable)
        else:
            raise NotImplementedError()

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
            raise NotImplementedError()

    @profile_led_blue.setter
    def profile_led_blue(self, enable: bool) -> None:
        """
        Set blue profile LED state

        :param enable: Status of blue profile LED
        :type enable: bool
        """
        if self.has('lighting_profile_led_blue'):
            self._dbus_interfaces['profile_led'].setBlueLED(enable)
        else:
            raise NotImplementedError()

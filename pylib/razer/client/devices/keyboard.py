import dbus as _dbus

from razer.client.constants import MACRO_LED_STATIC, MACRO_LED_BLINK
from razer.client.devices import RazerDevice as __RazerDevice


class RazerKeyboard(__RazerDevice):
    def __init__(self, serial, daemon_dbus=None):
        default_capabilities = {
            'game_mode_led': True,
            'macro_mode_led': True,
            'macro_mode_led_effect': True,
        }
        self._update_capabilities(default_capabilities)

        # Go get base stuff
        super(RazerKeyboard, self).__init__(serial, daemon_dbus)

        # Setup base stuff if need be
        if self.has('game_mode_led'):
            self._dbus_interfaces['game_mode_led'] = _dbus.Interface(self._dbus, "razer.device.led.gamemode")

        if self.has('macro_mode_led'):
            self._dbus_interfaces['macro_mode_led'] = _dbus.Interface(self._dbus, "razer.device.led.macromode")

    @property
    def game_mode_led(self):
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
    def game_mode_led(self, value):
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
    def macro_mode_led(self):
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
    def macro_mode_led(self, value):
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
    def macro_mode_led_effect(self):
        """
        Get macro mode LED effect

        Can get either blinking or static
        :return: Effect ID
        :rtype: int
        """
        if self.has('macro_mode_led_effect'):
            return self._dbus_interfaces['macro_mode_led_effect'].getMacroMode()
        else:
            return False

    @macro_mode_led.setter
    def macro_mode_led(self, value):
        """
        Set macro mode LED effect

        Can set to either MACRO_LED_STATIC or MACRO_LED_BLINK
        :param value: Effect ID
        :type value: int
        """
        if self.has('macro_mode_led_effect') and value in (MACRO_LED_STATIC, MACRO_LED_BLINK):
            self._dbus_interfaces['macro_mode_led_effect'].setMacroEffect(value)


# Just an example of disabling macro keys for the Ultimate 2016
# TODO create factory
class BlackWidowUltimate2016(RazerKeyboard):
    def __init__(self, serial, daemon_dbus=None):
        default_capabilities = {
            'game_mode_led': True,
            'macro_mode_led': False,
            'macro_mode_led_effect': False,
        }
        self._update_capabilities(default_capabilities)

        # Go get base stuff
        super(BlackWidowUltimate2016, self).__init__(serial, daemon_dbus)
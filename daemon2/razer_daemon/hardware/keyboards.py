"""
Keyboards class
"""
import razer_daemon.dbus_services.dbus_methods
from razer_daemon.hardware.device_base import RazerDevice


class RazerBlackWidow2013(RazerDevice):
    """
    Class for the BlackWidow Ultimate 2013
    """
    USB_VID = 0x1532
    USB_PID = 0x011A
    METHODS = ['get_firmware', 'get_brightness', 'enable_macro_keys', 'set_brightness', 'get_device_type', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static']

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = razer_daemon.dbus_services.dbus_methods.get_brightness(self)
        razer_daemon.dbus_services.dbus_methods.set_brightness(self, 0)

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        brightness = self.suspend_args.get('brightness', 100)
        razer_daemon.dbus_services.dbus_methods.set_brightness(self, brightness)

import dbus as _dbus

from openrazer.client.devices import RazerDevice as __RazerDevice
from openrazer.client.macro import RazerMacro as _RazerMacro
from openrazer.client import constants as _c


class RazerMouse(__RazerDevice):
    _MACRO_CLASS = _RazerMacro

    def __init__(self, serial, vid_pid=None, daemon_dbus=None):
        super(RazerMouse, self).__init__(serial, vid_pid=vid_pid, daemon_dbus=daemon_dbus)

        # Capabilities
        self._capabilities['poll_rate'] = self._has_feature('razer.device.misc', ('getPollRate', 'setPollRate'))
        self._capabilities['dpi'] = self._has_feature('razer.device.dpi', ('getDPI', 'setDPI'))

        if self.has('dpi'):
            self._dbus_interfaces['dpi'] = _dbus.Interface(self._dbus, "razer.device.dpi")

    @property
    def max_dpi(self) -> int:
        """
        Gets max DPI

        :return: Max DPI, if device does not have DPI it'll return None
        :rtype: int or None
        """
        if self.has('dpi'):
            return int(self._dbus_interfaces['dpi'].maxDPI())
        else:
            return None

    @property
    def dpi(self) -> tuple:
        """
        Get mouse DPI

        Will return a tuple
        :return: DPI (500, 500)
        :rtype: tuple

        :raises NotImplementedError: If function is not supported
        """
        if self.has('dpi'):
            dpi_x, dpi_y = self._dbus_interfaces['dpi'].getDPI()
            # Converting to integers to remove the dbus types
            return int(dpi_x), int(dpi_y)
        else:
            raise NotImplementedError()

    @dpi.setter
    def dpi(self, value:tuple):
        """
        Set mouse dpi

        Daemon does type validation but cant be too careful
        :param value: DPI X, Y tuple
        :type value: tuple

        :raises ValueError: If the tuple isnt long enough or contains invalid crap
        :raises NotImplementedError: If function is not supported
        """
        if self.has('dpi'):
            if len(value) != 2:
                raise ValueError("DPI tuple is not of length 2. Length: {0}".format(len(value)))
            dpi_x, dpi_y = value

            if not isinstance(dpi_x, int) or not isinstance(dpi_y, int):
                raise ValueError("DPI X or Y is not an integer, X:{0} Y:{1}".format(type(dpi_x), type(dpi_y)))

            if dpi_x < 0 or dpi_x > 16000: # TODO add in max dpi option
                raise ValueError("DPI X either too small or too large, X:{0}".format(dpi_x))
            if dpi_y < 0 or dpi_y > 16000:  # TODO add in max dpi option
                raise ValueError("DPI Y either too small or too large, Y:{0}".format(dpi_y))

            self._dbus_interfaces['dpi'].setDPI(dpi_x, dpi_y)
        else:
            raise NotImplementedError()

    @property
    def poll_rate(self) -> int:
        """
        Get poll rate from device

        :return: Poll rate
        :rtype: int

        :raises NotImplementedError: If function is not supported
        """
        if self.has('poll_rate'):
            return int(self._dbus_interfaces['device'].getPollRate())
        else:
            raise NotImplementedError()


    @poll_rate.setter
    def poll_rate(self, poll_rate:int):
        """
        Set poll rate of device

        :param poll_rate: Polling rate
        :type poll_rate: int

        :raises NotImplementedError: If function is not supported
        """
        if self.has('poll_rate'):
            if not isinstance(poll_rate, int):
                raise ValueError("Poll rate is not an integer: {0}".format(poll_rate))
            if poll_rate not in (_c.POLL_125HZ, _c.POLL_500HZ, _c.POLL_1000HZ):
                raise ValueError('Poll rate "{0}" is not one of {1}'.format(poll_rate, (_c.POLL_125HZ, _c.POLL_500HZ, _c.POLL_1000HZ)))

            self._dbus_interfaces['device'].setPollRate(poll_rate)

        else:
            raise NotImplementedError()

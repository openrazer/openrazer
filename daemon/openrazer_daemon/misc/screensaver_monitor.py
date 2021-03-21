"""
Screensaver class which watches dbus signals to see if screensaver is active
"""
import logging
import dbus
import dbus.exceptions

DBUS_SCREENSAVER_INTERFACES = (
    'org.cinnamon.ScreenSaver',
    'org.freedesktop.ScreenSaver',
    'org.gnome.ScreenSaver',
    'org.mate.ScreenSaver',
    'org.xfce.ScreenSaver',
)


class ScreensaverMonitor(object):
    """
    Simple class for monitoring signals on the Session Bus
    """

    def __init__(self, parent):
        self._logger = logging.getLogger('razer.screensaver')
        self._logger.info("Initialising DBus Screensaver Monitor")

        self._parent = parent
        self._monitoring = True
        self._active = None

        self._dbus_instances = []
        # Get session bus
        bus = dbus.SessionBus()
        # Loop through and monitor the signals
        for screensaver_interface in DBUS_SCREENSAVER_INTERFACES:
            bus.add_signal_receiver(self.signal_callback, dbus_interface=screensaver_interface, signal_name='ActiveChanged')

    @property
    def monitoring(self):
        """
        Monitoring property, if true then suspend/resume will be actioned.

        :return: If monitoring
        :rtype: bool
        """
        return self._monitoring

    @monitoring.setter
    def monitoring(self, value):
        """
        Monitoring property setter, if true then suspend/resume will be actioned.

        :param value: If monitoring
        :type: bool
        """
        self._monitoring = bool(value)

    def suspend(self):
        """
        Suspend the device
        """
        self._logger.debug("Received screensaver active signal")
        self._parent.suspend_devices()

    def resume(self):
        """
        Resume the device
        """
        self._logger.debug("Received screensaver inactive signal")
        self._parent.resume_devices()

    def signal_callback(self, active):
        """
        Called by DBus when a signal is found

        :param active: If the screensaver is active
        :type active: dbus.Boolean
        """
        active = bool(active)

        if self.monitoring:
            if active:
                # Only trigger once per state change
                if self._active is None or not self._active:
                    self._active = active
                    self.suspend()
            else:
                if self._active is None or self._active:
                    self._active = active
                    self.resume()

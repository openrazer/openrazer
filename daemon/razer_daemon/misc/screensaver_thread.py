"""
Screensaver thread which watches dbus to see if screensaver is active
"""
import logging
import threading
import time
import dbus
import dbus.exceptions

DBUS_OPTIONS = (
    ('org.freedesktop.ScreenSaver', '/org/freedesktop/ScreenSaver', 'org.freedesktop.ScreenSaver'),
    ('com.canonical.Unity', '/org/gnome/ScreenSaver', 'org.gnome.ScreenSaver'),
    ('org.mate.ScreenSaver', '/org/mate/ScreenSaver', 'org.mate.ScreenSaver')
)


class ScreensaverThread(threading.Thread):
    """
    ScreensaverThread

    This thread polls (com.canonical.Unity, /org/gnome/ScreenSaver, org.gnome.ScreenSaver) to see
    if the screensaver is active, if its inactive it will call the parent function to suspend all
    the devices
    """
    def __init__(self, parent, active=True):
        super(ScreensaverThread, self).__init__()

        self.logger = logging.getLogger('razer.screensaver')
        self.logger.info("Initialising DBus Screensaver Thread")

        self._active = active
        self._shutdown = False
        self._dbus_interface = None
        self._parent = parent

        self._try_count = 0

    def load_dbus(self):
        """
        Setup the connection to DBUS
        """
        self.logger.info("Initialising DBus screensaver object")

        try:
            session_bus = dbus.SessionBus()

            for bus_name, object_path, interface_name in DBUS_OPTIONS:
                try:
                    screensaver_object = session_bus.get_object(bus_name, object_path)
                    self._dbus_interface = dbus.Interface(screensaver_object, interface_name)
                    self._dbus_interface.GetActive()
                    break
                except dbus.exceptions.DBusException:
                    # Skip over exceptions as we only care if we find a valid endpoint.
                    pass
            else:
                self.logger.warning("Could not find screensaver DBus")
                self._dbus_interface = None
                self._try_count = 5

        except dbus.exceptions.DBusException as err:
            self.logger.exception("Caught exception whilst trying to get screensaver DBus", exc_info=err)
            self._try_count += 1
            self._dbus_interface = None

        if self._try_count >= 3:
            self.logger.warning("Giving up on screensaver")

    @property
    def active(self):
        """
        Screensaver thread active

        This property defines wether or not the screensaver function is active
        :return: Active
        :rtype: bool
        """
        return self._active
    @active.setter
    def active(self, active_state):
        """
        Screensaver thread active

        :param active_state: Active
        :type active_state: bool
        """
        self._active = active_state
    @property
    def shutdown(self):
        """
        Threads loop condition

        Set to true to stop the thread
        :return: Shutdown state
        :rtype: bool
        """
        return self._shutdown
    @shutdown.setter
    def shutdown(self, state):
        """
        Thread loop condition

        :param state: Shutdown state
        :type state: bool
        """
        self._shutdown = state

    def run(self):
        """
        Thread run loop
        """
        suspended = False

        if self._dbus_interface is None:
            # Sleep for a while as it takes time for screensaver to start
            time.sleep(2)
            self.load_dbus()

        while not self._shutdown:
            if self._active and self._try_count < 3:
                try:
                    if self._dbus_interface is None:
                        self.load_dbus()
                    else:
                        screensaver_active = self._dbus_interface.GetActive()
                        if screensaver_active:
                            # Screensaver is active

                            if not suspended:
                                suspended = True
                                self.logger.info("Suspend screensaver")
                                self._parent.suspend_devices()
                        else:
                            if suspended:
                                suspended = False
                                self.logger.info("Resume screensaver")
                                self._parent.resume_devices()
                    # pylint: disable=broad-except
                except Exception as err:
                    self.logger.exception("Caught exception in run loop", exc_info=err)
                    self._dbus_interface = None
                    self._try_count = 4


            # Sleep
            time.sleep(0.2)

        self.logger.info("Screensaver Thread finished")

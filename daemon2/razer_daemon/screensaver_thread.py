import dbus
import threading
import time
import logging


class ScreensaverThread(threading.Thread):
    def __init__(self, parent, active=True):
        super(ScreensaverThread, self).__init__()

        self.logger = logging.getLogger('razer.screensaver')
        self.logger.info("Initialising DBus Screensaver Thread")

        self._active = active
        self._shutdown = False
        self._dbus_interface = None
        self._parent = parent

        self.load_dbus()

    def load_dbus(self):
        self.logger.info("Initialising DBus Objects")
        session_bus = dbus.SessionBus()
        unity_object = session_bus.get_object('com.canonical.Unity', '/org/gnome/ScreenSaver')
        self._dbus_interface = dbus.Interface(unity_object, 'org.gnome.ScreenSaver')

    @property
    def active(self):
        return self._active
    @active.setter
    def active(self, active_state):
        self._active = active_state
    @property
    def shutdown(self):
        return self._shutdown
    @shutdown.setter
    def shutdown(self, state):
        self._shutdown = state

    def run(self):
        suspended = False

        while not self._shutdown:
            if self._active:
                try:
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

                except Exception as err:
                    self.logger.exception("Caught exception in run loop", exc_info=err)

            time.sleep(0.1)
        self.logger.info("Screensaver Thread finished")
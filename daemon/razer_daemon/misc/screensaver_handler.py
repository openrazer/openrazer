"""
Watches dbus to see if screensaver is active
"""
import logging
import dbus
import dbus.exceptions

DBUS_OPTIONS = (
    ('org.freedesktop.ScreenSaver', '/org/freedesktop/ScreenSaver', 'org.freedesktop.ScreenSaver'),
    ('com.canonical.Unity', '/org/gnome/ScreenSaver', 'org.gnome.ScreenSaver'),
    ('org.gnome.ScreenSaver', '/org/gnome/ScreenSaver', 'org.gnome.ScreenSaver'),
    ('org.mate.ScreenSaver', '/org/mate/ScreenSaver', 'org.mate.ScreenSaver'),
)


class ScreensaverHandler:
    """
    ScreensaverThread

    This handler watches (com.canonical.Unity, /org/gnome/ScreenSaver, org.gnome.ScreenSaver) to see
    if the screensaver is active, if its inactive it will call the parent function to suspend all
    the devices
    """
    def __init__(self, parent, watch_enabled=True):

        self.logger = logging.getLogger('razer.screensaver')
        self.logger.info("Initialising DBus screensaver handler")

        self._parent = parent

        self._watch_enabled = watch_enabled
        self._active = False
        self.connection = None
        self._instances = []

        self._load_dbus()

    @property
    def watch_enabled(self):
        return self._watch_enabled

    @watch_enabled.setter
    def watch_enabled(self, enabled):
        self._watch_enabled = enabled

    def _load_dbus(self):
        """
        Setup the connection to DBUS
        """
        self._dbus = dbus.SessionBus()

        for bus_name, object_path, interface_name in DBUS_OPTIONS:
            self._instances.append(ScreensaverInstance(self, self._dbus,
                                                       bus_name, object_path, interface_name))

    @property
    def active(self):
        return self._active

    @active.setter
    def active(self, state):
        if state == self._active:
            return
        self._active = state

        if self._watch_enabled is False:
            return

        if state:
            self.logger.info("Screensaver active, suspend devices")
            self._parent.suspend_devices()
        else:
            self.logger.info("Screensaver inactive, resume devices")
            self._parent.resume_devices()


class ScreensaverInstance:
    def __init__(self, parent, session_bus, bus_name, object_path, interface_name):
        self.logger = logging.getLogger('razer.screensaver')
        self._parent = parent
        self._dbus = session_bus
        self._bus_name = bus_name
        self._object_path = object_path
        self._interface_name = interface_name
        self._dbus_interface = None
        self._dbus.watch_name_owner(self._bus_name, self._service_available_callback)

    def _connect(self):
        def callback(value):
            self._parent.active = value

        if self._parent.connection is not None:
            return False

        try:
            if self._dbus.name_has_owner(self._bus_name) is False:
                self.logger.debug("No owner found for name %s", self._bus_name)
                return False

            screensaver_object = self._dbus.get_object(self._bus_name, self._object_path)
            self._dbus_interface = dbus.Interface(screensaver_object, self._interface_name)
            self._dbus_interface.connect_to_signal('ActiveChanged', callback)
            self._parent.active = self._dbus_interface.GetActive()
            self._parent.connection = self._bus_name
            self.logger.info("Found screensaver: %s", self._bus_name)

        except dbus.exceptions.DBusException:
            return False

        return True

    def _service_available_callback(self, changed):
        if self._parent.connection == self._bus_name and changed is '':
            # Service went away!
            self.logger.info("Screensaver %s went away!", self._bus_name)
            self._parent.connection = None
            self._dbus_interface = None

        elif self._parent.connection is None and changed is not '':
            self._connect()



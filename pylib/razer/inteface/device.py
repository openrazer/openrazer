import dbus
import logging
from xml.etree import ElementTree as ET

DAEMON_BUS_NAME = 'org.razer'
DEVICE_OBJECT_PATH = '/org/razer/device/'



class Device(object):
    def __init__(self, device_id):
        session_bus = dbus.SessionBus()
        self._dbus = session_bus.get_object(DAEMON_BUS_NAME, DEVICE_OBJECT_PATH + device_id)

        introspect_interface = dbus.Interface(self._dbus, 'org.freedesktop.DBus.Introspectable')
        xml_spec = introspect_interface.Introspect()
        root = ET.fromstring(xml_spec)

        self.interfaces = {}

        for child in root:

            if child.tag != 'interface' or child.attrib.get('name') == 'org.freedesktop.DBus.Introspectable':
                continue

            current_interface = child.attrib['name']
            current_interface_methods = []

            for method in child:
                if method.tag != 'method':
                    continue

                current_interface_methods.append(method.attrib.get('name'))

            if current_interface in INTERFACE_MAP:
                dbus_object = dbus.Interface(self._dbus, current_interface)
                self.interfaces[current_interface] = INTERFACE_MAP[current_interface](current_interface_methods, dbus_object)
            else:
                logging.warning("Interface: {0} doesn't have a corrosponding class".format(current_interface))



class NoSuchMethodException(Exception):
    pass

class Interface(object):
    def __init__(self, methods, dbus_object):
        self._methods = methods
        self._dbus = dbus_object

    def __getattribute__(self, item):
        if item.startswith('_'):
            return super(Interface, self).__getattribute__(item)

        if item in self._methods:
            return super(Interface, self).__getattribute__(item)
        else:
            raise NoSuchMethodException("Method {0} does not exist in this context".format(item))

class LEDGameModeInterface(Interface):
    def getGameMode(self):
        return self._dbus.getGameMode()
    def setGameMode(self, value):
        if isinstance(value, bool):
            self._dbus.setGameMode(value)
        else:
            raise ValueError("Game mode value must be boolean")

class LEDMacroModeInterface(Interface):
    def getMacroMode(self):
        return self._dbus.getMacroMode()
    def setMacroMode(self, value):
        if isinstance(value, bool):
            self._dbus.setMacroMode(value)
        else:
            raise ValueError("Macro mode value must be boolean")
    def getMacroEffect(self):
        return self._dbus.getMacroEffect()
    def setMacroEffect(self, value):
        self._dbus.setMAcroEffect(value)

class LightingBrightness(Interface):
    def getBrightness(self):
        return self._dbus.getBrightness()
    def setBrightness(self, value):
        if isinstance(value, (float, int)) and 0 <= value <= 100:
            self._dbus.setBrightness(value)
        else:
            raise ValueError("Brightness must be between 0-100")


INTERFACE_MAP = {
    'razer.device.led.gamemode': LEDGameModeInterface,
    'razer.device.led.macromode': LEDMacroModeInterface,
    'razer.device.lighting.brightness': LightingBrightness
}
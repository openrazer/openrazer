# SPDX-License-Identifier: GPL-2.0-or-later
"""OpenRazer-compatible D-Bus surface for validated Basilisk Bluetooth features."""
import logging
import dbus.service
from openrazer_daemon.dbus_services.service import DBusService

MISC = 'razer.device.misc'
POWER = 'razer.device.power'
DPI = 'razer.device.dpi'
CHROMA = 'razer.device.lighting.chroma'
BRIGHTNESS = 'razer.device.lighting.brightness'
LOGO = 'razer.device.lighting.logo'
SCROLL = 'razer.device.lighting.scroll'


class BasiliskV3ProBluetooth(DBusService):
    USB_VID = 0x068E  # Bluetooth PnP namespace, not the USB VID.
    USB_PID = 0x00AC
    METHODS = []
    ZONES = ()  # Hardware owns state; do not restore daemon persistence on reconnect.
    DEVICE_IMAGE = 'https://dl.razerzone.com/src2/6220/6220-4-en-v1.png'

    def __init__(self, backend, path, address):
        self.backend = backend
        self.path = path
        self.serial = 'BT_' + address.replace(':', '').upper()
        self.storage_name = self.serial
        self.additional_interfaces = []
        self.effect_sync = False
        self.parent = None
        self.logger = logging.getLogger('razer.bluetooth.' + self.serial)
        super().__init__('/org/razer/device/' + self.serial)

    def invoke(self, operation, *args):
        return self.backend.call(self.backend.perform(self.path, operation, *args))

    def register_parent(self, parent):
        self.parent = parent

    def notify(self, msg):
        # Only forward actions this transport has actually validated.
        if not self.effect_sync or not msg or msg[0] != 'effect':
            return
        try:
            if msg[2] == 'setStatic':
                self.setStatic(*msg[3:6])
            elif msg[2] == 'setBrightness':
                # USB effect events contain brightness in 0..255, not percent.
                self.setBrightness(float(msg[3]) * 100 / 255)
        except Exception:
            self.logger.warning('Could not apply synchronized effect', exc_info=True)

    def suspend_device(self):
        # Keep hardware state; screensaver lighting restoration is not validated yet.
        pass

    def resume_device(self):
        pass

    def close(self):
        # Cleanup is owned by the shared backend; never query disconnected hardware.
        pass

    @dbus.service.method(MISC, out_signature='s')
    def getSerial(self):
        return self.serial

    @dbus.service.method(MISC, out_signature='s')
    def getDeviceName(self):
        return 'Razer Basilisk V3 Pro (Bluetooth)'

    @dbus.service.method(MISC, out_signature='s')
    def getDeviceType(self):
        return 'mouse'

    @dbus.service.method(MISC, out_signature='ai')
    def getVidPid(self):
        return [self.USB_VID, self.USB_PID]

    @dbus.service.method(MISC, out_signature='s')
    def getDriverVersion(self):
        from openrazer_daemon.daemon import __version__
        return __version__

    @dbus.service.method(MISC, out_signature='s')
    def getFirmware(self):
        return 'unknown'  # PnP revision is not a validated firmware version.

    @dbus.service.method(MISC, out_signature='s')
    def getDeviceImage(self):
        return self.DEVICE_IMAGE

    @dbus.service.method(MISC, out_signature='b')
    def hasDedicatedMacroKeys(self):
        return False

    @dbus.service.method(MISC, out_signature='b')
    def hasMatrix(self):
        return False

    @dbus.service.method(MISC, out_signature='ai')
    def getMatrixDimensions(self):
        return [0, 0]

    @dbus.service.method(POWER, out_signature='d')
    def getBattery(self):
        return self.invoke('battery')

    @dbus.service.method(POWER, out_signature='q')
    def getIdleTime(self):
        return self.invoke('idle')

    @dbus.service.method(POWER, in_signature='q')
    def setIdleTime(self, seconds):
        self.invoke('set_idle', seconds)

    @dbus.service.method(DPI, out_signature='i')
    def maxDPI(self):
        return 30000

    @dbus.service.method(DPI, out_signature='ai')
    def getDPI(self):
        return self.invoke('dpi')

    @dbus.service.method(DPI, in_signature='qq')
    def setDPI(self, x, y):
        self.invoke('set_dpi', x, y)

    @dbus.service.method(DPI, out_signature='(ya(qq))')
    def getDPIStages(self):
        active, stages = self.invoke('dpi_stages')
        return active + 1, stages

    @dbus.service.method(BRIGHTNESS, out_signature='d')
    def getBrightness(self):
        return self.invoke('brightness', 'backlight')

    @dbus.service.method(BRIGHTNESS, in_signature='d')
    def setBrightness(self, brightness):
        self.invoke('set_brightness', 'backlight', brightness)

    @dbus.service.method(CHROMA, in_signature='yyy')
    def setStatic(self, red, green, blue):
        # Whole-device effect, matching the existing chroma endpoint semantics.
        for zone in ('backlight', 'logo', 'scroll'):
            self.invoke('set_static', zone, red, green, blue)

    def effect(self, zone):
        raw = self.invoke('lighting', zone)
        return 'static' if raw[:4] == bytes([1, 0, 0, 1]) else 'unknown'

    def colors(self, zone):
        raw = self.invoke('lighting', zone)
        return list(raw[4:7]) if raw[:4] == bytes([1, 0, 0, 1]) else []

    @dbus.service.method(CHROMA, out_signature='s')
    def getEffect(self):
        return self.effect('backlight')

    @dbus.service.method(CHROMA, out_signature='ay')
    def getEffectColors(self):
        return self.colors('backlight')

    @dbus.service.method(LOGO, out_signature='d')
    def getLogoBrightness(self):
        return self.invoke('brightness', 'logo')

    @dbus.service.method(LOGO, in_signature='d')
    def setLogoBrightness(self, brightness):
        self.invoke('set_brightness', 'logo', brightness)

    @dbus.service.method(LOGO, in_signature='yyy')
    def setLogoStatic(self, red, green, blue):
        self.invoke('set_static', 'logo', red, green, blue)

    @dbus.service.method(LOGO, out_signature='s')
    def getLogoEffect(self):
        return self.effect('logo')

    @dbus.service.method(LOGO, out_signature='ay')
    def getLogoEffectColors(self):
        return self.colors('logo')

    @dbus.service.method(SCROLL, out_signature='d')
    def getScrollBrightness(self):
        return self.invoke('brightness', 'scroll')

    @dbus.service.method(SCROLL, in_signature='d')
    def setScrollBrightness(self, brightness):
        self.invoke('set_brightness', 'scroll', brightness)

    @dbus.service.method(SCROLL, in_signature='yyy')
    def setScrollStatic(self, red, green, blue):
        self.invoke('set_static', 'scroll', red, green, blue)

    @dbus.service.method(SCROLL, out_signature='s')
    def getScrollEffect(self):
        return self.effect('scroll')

    @dbus.service.method(SCROLL, out_signature='ay')
    def getScrollEffectColors(self):
        return self.colors('scroll')

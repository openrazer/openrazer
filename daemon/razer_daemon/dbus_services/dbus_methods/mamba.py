"""
BlackWidow Chroma Effects
"""
import math
import struct
from razer_daemon.dbus_services import endpoint

@endpoint('razer.device.power', 'getBattery', out_sig='d')
def get_battery(self):
    """
    Get mouse's battery level
    """
    self.logger.debug("DBus call get_battery")

    driver_path = self.get_driver_path('get_battery')

    with open(driver_path, 'r') as driver_file:
        battery_255 = float(driver_file.read().strip())
        if battery_255 < 0:
            return -1.0

        battery_100 = (battery_255 / 255) * 100
        return battery_100

@endpoint('razer.device.power', 'isCharging', out_sig='b')
def is_charging(self):
    """
    Get charging status
    """
    self.logger.debug("DBus call is_charging")

    driver_path = self.get_driver_path('is_charging')

    with open(driver_path, 'r') as driver_file:
        return bool(int(driver_file.read().strip()))

@endpoint('razer.device.power', 'setIdleTime', in_sig='q')
def set_idle_time(self, idle_time):
    """
    Set the idle time of the mouse in seconds

    :param idle_time: Idle time in seconds (unsigned short)
    :type idle_time: int
    """
    self.logger.debug("DBus call set_idle_time")

    driver_path = self.get_driver_path('set_idle_time')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(idle_time))

@endpoint('razer.device.power', 'setLowBatteryThreshold', in_sig='y')
def set_low_battery_threshold(self, threshold):
    """
    Set the low battery threshold as a percentage

    :param threshold: Battery threshold as a percentage
    :type threshold: int
    """
    self.logger.debug("DBus call set_low_battery_threshold")

    driver_path = self.get_driver_path('set_idle_time')

    threshold = math.floor((threshold/100) * 255)

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(threshold))

@endpoint('razer.device.lighting.power', 'setChargeEffect', in_sig='y')
def set_charge_effect(self, charge_effect):
    """
    Set the charging effect.

    If 0x00 then it will use the current mouse's effect
    If 0x01 it will use the charge colour

    :param charge_effect: Charge effect
    :type charge_effect: int
    :return:
    """
    self.logger.debug("DBus call set_charge_effect")

    driver_path = self.get_driver_path('set_charging_effect')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes([charge_effect]))

@endpoint('razer.device.lighting.power', 'setChargeColour', in_sig='yyy')
def set_charge_colour(self, red, green, blue):
    """
    Set the charge colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_charge_colour")

    driver_path = self.get_driver_path('set_charging_colour')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)

@endpoint('razer.device.dpi', 'setDPI', in_sig='qq')
def set_dpi_xy(self, dpi_x, dpi_y):
    """
    Set the DPI on the mouse, Takes in 4 bytes big-endian

    :param dpi_x: X DPI
    :type dpi_x: int
    :param dpi_y: Y DPI
    :type dpi_x: int
    """
    self.logger.debug("DBus call set_dpi_both")

    driver_path = self.get_driver_path('set_mouse_dpi')

    dpi_bytes = struct.pack('>HH', dpi_x, dpi_y)

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(dpi_bytes)

@endpoint('razer.device.misc', 'setPollRate', in_sig='q')
def set_poll_rate(self, rate):
    """
    Set the DPI on the mouse, Takes in 4 bytes big-endian

    :param rate: Poll rate
    :type rate: int
    """
    self.logger.debug("DBus call set_poll_rate")

    if rate in (1000, 500, 128):
        driver_path = self.get_driver_path('poll_rate')

        with open(driver_path, 'w') as driver_file:
            driver_file.write(str(rate))
    else:
        self.logger.error("Poll rate %d is invalid", rate)

@endpoint('razer.device.lighting.logo', 'getLogoActive', out_sig='b')
def get_logo_active(self):
    """
    Get if the logo is light up

    :return: Active
    :rtype: bool
    """
    self.logger.debug("DBus call get_logo_active")

    driver_path = self.get_driver_path('mode_logo')

    with open(driver_path, 'r') as driver_file:
        active = int(driver_file.read().strip())
        return active == 1

@endpoint('razer.device.lighting.logo', 'setLogoActive', in_sig='b')
def set_logo_active(self, active):
    """
    Get if the logo is light up

    :param active: Is active
    :type active: bool
    """
    self.logger.debug("DBus call set_logo_active")

    driver_path = self.get_driver_path('mode_logo')

    with open(driver_path, 'w') as driver_file:
        if active:
            driver_file.write('1')
        else:
            driver_file.write('0')

@endpoint('razer.device.lighting.logo', 'setLogo', in_sig='yyy')
def set_te_logo(self, red, green, blue):
    """
    Set the red green blue of logo

    :param red: Red component
    :type red: int
    """
    self.logger.debug("DBus call set_logo_active")

    driver_path = self.get_driver_path('mode_logo')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.scroll', 'getScrollActive', out_sig='b')
def get_scroll_active(self):
    """
    Get if the scroll is light up

    :return: Active
    :rtype: bool
    """
    self.logger.debug("DBus call get_scroll_active")

    driver_path = self.get_driver_path('mode_scroll')

    with open(driver_path, 'r') as driver_file:
        active = int(driver_file.read().strip())
        return active == 1

@endpoint('razer.device.lighting.scroll', 'setScrollActive', in_sig='b')
def set_scroll_active(self, active):
    """
    Get if the scroll is light up

    :param active: Is active
    :type active: bool
    """
    self.logger.debug("DBus call set_scroll_active")

    driver_path = self.get_driver_path('mode_scroll')

    with open(driver_path, 'w') as driver_file:
        if active:
            driver_file.write('1')
        else:
            driver_file.write('0')




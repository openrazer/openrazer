# SPDX-License-Identifier: GPL-2.0-or-later

import math
import struct
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.power', 'getBattery', out_sig='d')
def get_battery(self):
    """
    Get mouse's battery level
    """
    self.logger.debug("DBus call get_battery")

    driver_path = self.get_driver_path('charge_level')

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

    driver_path = self.get_driver_path('charge_status')

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

    driver_path = self.get_driver_path('device_idle_time')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(idle_time))


@endpoint('razer.device.power', 'getIdleTime', out_sig='q')
def get_idle_time(self):
    """
    Get the idle time of the mouse in seconds

    :return: Idle time in seconds (unsigned short)
    :rtype: int
    """
    self.logger.debug("DBus call get_idle_time")

    driver_path = self.get_driver_path('device_idle_time')

    with open(driver_path, 'r') as driver_file:
        result = driver_file.read()
        result = int(result.strip())

    return result


@endpoint('razer.device.power', 'setLowBatteryThreshold', in_sig='y')
def set_low_battery_threshold(self, threshold):
    """
    Set the low battery threshold as a percentage

    :param threshold: Battery threshold as a percentage
    :type threshold: int
    """
    self.logger.debug("DBus call set_low_battery_threshold")

    driver_path = self.get_driver_path('charge_low_threshold')

    threshold = math.floor((threshold / 100) * 255)

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(threshold))


@endpoint('razer.device.power', 'getLowBatteryThreshold', out_sig='y')
def get_low_battery_threshold(self):
    """
    Get the low battery threshold as a percentage

    :return: Battery threshold as a percentage
    :rtype: int
    """
    self.logger.debug("DBus call get_low_battery_threshold")

    driver_path = self.get_driver_path('charge_low_threshold')

    with open(driver_path, 'r') as driver_file:
        result = driver_file.read()
        result = int(result.strip())

    return round((result / 255) * 100)


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

    driver_path = self.get_driver_path('charge_effect')

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

    driver_path = self.get_driver_path('charge_colour')

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
    self.logger.debug("DBus call set_dpi_xy")

    if 'available_dpi' in self.METHODS:
        if dpi_y > 0:
            raise RuntimeError("Devices with available_dpi are expected to have only one DPI value set, got " + str(dpi_x) + ", " + str(dpi_y))
        if dpi_x not in self.AVAILABLE_DPI:
            raise RuntimeError("Provided DPI " + str(dpi_x) + " is not in available_dpi values: " + str(self.AVAILABLE_DPI))

    # check that DPI is not more than the maximum
    if dpi_x > self.DPI_MAX:
        raise RuntimeError("Provided DPI " + str(dpi_x) + " is larger than maximum of " + str(self.DPI_MAX))
    if dpi_y > self.DPI_MAX:
        raise RuntimeError("Provided DPI " + str(dpi_x) + " is larger than maximum of " + str(self.DPI_MAX))

    driver_path = self.get_driver_path('dpi')

    if self._testing:
        with open(driver_path, 'w') as driver_file:
            if dpi_y <= 0:
                driver_file.write("{}".format(dpi_x))
            else:
                driver_file.write("{}:{}".format(dpi_x, dpi_y))
        return

    # If the application requests just one value to be written
    if dpi_y <= 0:
        dpi_bytes = struct.pack('>H', dpi_x)
    else:
        dpi_bytes = struct.pack('>HH', dpi_x, dpi_y)

    # store to local variable (TODO: can we replace this by getting from persistence?)
    self.dpi[0] = dpi_x
    self.dpi[1] = dpi_y

    self.set_persistence(None, "dpi_x", dpi_x)
    self.set_persistence(None, "dpi_y", dpi_y)

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(dpi_bytes)


@endpoint('razer.device.dpi', 'getDPI', out_sig='ai')
def get_dpi_xy(self):
    """
    get the DPI on the mouse

    :return: List of X, Y DPI
    :rtype: list of int
    """
    self.logger.debug("DBus call get_dpi_xy")

    driver_path = self.get_driver_path('dpi')

    # try retrieving DPI from the hardware.
    # if we can't (e.g. because the mouse has been disconnected)
    # return the value in local storage.
    try:
        with open(driver_path, 'r') as driver_file:
            result = driver_file.read()
            dpi = [int(dpi) for dpi in result.strip().split(':')]
    except FileNotFoundError:
        return self.dpi

    if 'available_dpi' in self.METHODS:
        if len(dpi) != 1:
            raise RuntimeError("Devices with available_dpi are expected to have only one DPI value returned from driver, got " + str(dpi))
        dpi = dpi[0], 0

    return dpi


@endpoint('razer.device.dpi', 'setDPIStages', in_sig='ya(qq)')
def set_dpi_stages(self, active_stage, dpi_stages):
    """
    Set the DPI on the mouse, Takes in pairs of 2 bytes big-endian

    :param active_stage: DPI stage to enable
    :param dpi_stages: pairs of dpi X and dpi Y for each stage
    :type dpi_stages: list of (int, int)
    """
    self.logger.debug("DBus call set_dpi_stages")

    driver_path = self.get_driver_path('dpi_stages')

    dpi_bytes = struct.pack('B', active_stage)
    for dpi_x, dpi_y in dpi_stages:
        dpi_bytes += struct.pack('>HH', dpi_x, dpi_y)

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(dpi_bytes)


@endpoint('razer.device.dpi', 'getDPIStages', out_sig='(ya(qq))')
def get_dpi_stages(self):
    """
    get the DPI stages on the mouse

    :return: List of X, Y DPI
    :rtype: (int, list of (int, int))
    """
    self.logger.debug("DBus call get_dpi_stages")

    driver_path = self.get_driver_path('dpi_stages')

    dpi_stages = []
    with open(driver_path, 'rb') as driver_file:
        result = driver_file.read()

        (active_stage,) = struct.unpack('B', result[:1])
        result = result[1:]

        while len(result) >= 4:
            (dpi_x, dpi_y) = struct.unpack('>HH', result[:4])
            dpi_stages.append((dpi_x, dpi_y))
            result = result[4:]

    return (active_stage, dpi_stages)


@endpoint('razer.device.dpi', 'maxDPI', out_sig='i')
def max_dpi(self):
    self.logger.debug("DBus call max_dpi")

    return self.DPI_MAX


@endpoint('razer.device.dpi', 'availableDPI', out_sig='ai')
def available_dpi(self):
    self.logger.debug("DBus call available_dpi")

    if hasattr(self, 'AVAILABLE_DPI'):
        return self.AVAILABLE_DPI

    return []


@endpoint('razer.device.misc', 'setPollRate', in_sig='q')
def set_poll_rate(self, rate):
    """
    Set the polling rate on the device, Takes in 4 bytes big-endian

    :param rate: Poll rate
    :type rate: int
    """
    self.logger.debug("DBus call set_poll_rate")

    if rate not in self.POLL_RATES:
        raise RuntimeError("Poll rate " + str(rate) + " is not allowed. Allowed values: " + str(self.POLL_RATES))

    driver_path = self.get_driver_path('poll_rate')

    # remember poll rate
    self.poll_rate = rate

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(rate))


@endpoint('razer.device.misc', 'getPollRate', out_sig='i')
def get_poll_rate(self):
    """
    Get the polling rate from the device

    :return: Poll rate
    :rtype: int
    """
    self.logger.debug("DBus call get_poll_rate")

    return int(self.poll_rate)


@endpoint('razer.device.misc', 'getSupportedPollRates', out_sig='aq')
def get_supported_poll_rates(self):
    """
    Get the polling rates supported by the device

    :return: Supported poll rates
    :rtype: list of int
    """
    self.logger.debug("DBus call get_supported_poll_rates")

    return self.POLL_RATES


@endpoint('razer.device.misc', 'setHyperPollingLED', in_sig='y')
def set_hyperpolling_wireless_dongle_indicator_led_mode(self, mode):
    """
    Set the function of the LED on the dongle, takes in 1 char
    1 = Connection Status (green if connected to mouse)
    2 = Battery Status (green if high battery, yellow if medium battery, red if low battery)
    3 = Battery Warning (red if low battery, off otherwise)

    :param mode: LED mode
    :type mode: char
    """
    self.logger.debug("DBus call set_hyperpolling_wireless_dongle_indicator_led_mode")

    driver_path = self.get_driver_path('hyperpolling_wireless_dongle_indicator_led_mode')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(mode))


@endpoint('razer.device.misc', 'setHyperPollingPair', in_sig='s')
def set_hyperpolling_wireless_dongle_pair(self, pid):
    """
    Set Pairing mode, takes in 1 string which is the PID

    :param pid: product id
    :type pid: char
    """
    self.logger.debug("DBus call set_hyperpolling_wireless_dongle_pair")

    driver_path = self.get_driver_path('hyperpolling_wireless_dongle_pair')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(pid)


@endpoint('razer.device.misc', 'setHyperPollingUnpair', in_sig='s')
def set_hyperpolling_wireless_dongle_unpair(self, pid):
    """
    Set Unpairing mode, takes in 1 string which is the PID

    :param pid: product id
    :type pid: char
    """
    self.logger.debug("DBus call set_hyperpolling_wireless_dongle_unpair")

    driver_path = self.get_driver_path('hyperpolling_wireless_dongle_unpair')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(pid)

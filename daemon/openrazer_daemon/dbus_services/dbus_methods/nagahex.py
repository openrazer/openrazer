"""
Naga Hex DPI
"""
import struct
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.dpi', 'setDPI', in_sig='qq')
def set_dpi_xy_byte(self, dpi_x, dpi_y):
    """
    Set the DPI on the mouse, Takes in 4 bytes big-endian and converts it to bytes

    :param dpi_x: X DPI
    :type dpi_x: int
    :param dpi_y: Y DPI
    :type dpi_x: int
    """
    self.logger.debug("DBus call set_dpi_xy_byte")

    driver_path = self.get_driver_path('dpi')

    if dpi_x > 6750:
        dpi_x = 6750
    elif dpi_x < 100:
        dpi_x = 100
    if dpi_y > 6750:
        dpi_y = 6750
    elif dpi_y < 100:
        dpi_y = 100

    dpi_x_scaled = int(round(dpi_x / 6750 * 255, 2))
    dpi_y_scaled = int(round(dpi_y / 6750 * 255, 2))

    self.dpi[0] = dpi_x
    self.dpi[1] = dpi_y

    self.set_persistence(None, "dpi_x", dpi_x_scaled)
    self.set_persistence(None, "dpi_y", dpi_y_scaled)

    if self._testing:
        with open(driver_path, 'w') as driver_file:
            driver_file.write("{}:{}".format(dpi_x_scaled, dpi_y_scaled))
        return

    dpi_bytes = struct.pack('>BB', dpi_x_scaled, dpi_y_scaled)

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(dpi_bytes)


@endpoint('razer.device.dpi', 'getDPI', out_sig='ai')
def get_dpi_xy_byte(self):
    """
    get the DPI on the mouse

    :return: List of X, Y DPI
    :rtype: list of int
    """
    self.logger.debug("DBus call get_dpi_xy_byte")

    driver_path = self.get_driver_path('dpi')

    # try retrieving DPI from the hardware.
    # if we can't (e.g. because the mouse has been disconnected)
    # return the value in local storage.
    try:
        with open(driver_path, 'r') as driver_file:
            result = driver_file.read()
            dpi_x, dpi_y = [int(dpi) for dpi in result.strip().split(':')]
        dpi_x = int(round(dpi_x / 255 * 6750, 2))
        dpi_y = int(round(dpi_y / 255 * 6750, 2))
    except FileNotFoundError:
        dpi_x, dpi_y = self.dpi

    return [dpi_x, dpi_y]

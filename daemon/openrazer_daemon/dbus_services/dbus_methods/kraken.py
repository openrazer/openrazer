"""
Module for kraken methods
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.kraken', 'getCurrentEffect', out_sig='y')
def get_current_effect_kraken(self):
    """
    Get the device's current effect

    :return: The internal bitfield like 05 (in hex)
    :rtype: int
    """
    self.logger.debug("DBus call matrix_current_effect")

    driver_path = self.get_driver_path('matrix_current_effect')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip(), 16)


@endpoint('razer.device.lighting.kraken', 'getStaticArgs', out_sig='ai')
def get_static_effect_args_kraken(self):
    """
    Get the static effect arguments

    :return: List of args used for static effect
    :rtype: int
    """
    self.logger.debug("DBus call get_static_effect_args")

    driver_path = self.get_driver_path('matrix_effect_static')

    with open(driver_path, 'rb') as driver_file:
        bytestring = driver_file.read()
        if len(bytestring) != 4:
            raise ValueError("Response from driver is not valid, should be length 4 got: {0}".format(len(bytestring)))
        else:
            return list(bytestring[:3])  # We cut off the intensity value in the end, aint letting people mess with that.


@endpoint('razer.device.lighting.kraken', 'getBreathArgs', out_sig='ai')
def get_breath_effect_args_kraken(self):
    """
    Get the breath effect arguments

    :return: List of args used for breathing effect
    :rtype: int
    """
    self.logger.debug("DBus call get_breath_effect_args")

    driver_path = self.get_driver_path('matrix_effect_breath')

    with open(driver_path, 'rb') as driver_file:
        bytestring = driver_file.read()
        if len(bytestring) % 4 != 0:
            raise ValueError("Response from driver is not valid, should be length 4 got: {0}".format(len(bytestring)))
        else:
            result = []

            # Result could be 4 bytes (breathing1), 8 bytes (breathing2), 12 bytes (breathing3) and we need to cut of the
            # intensity value, so i thought it easier to cut it into chunks of 4, append the first 3 values to `result`
            for chunk in [bytestring[i:i+4] for i in range(0, len(bytestring), 4)]:
                # Get first 3 values
                values = list(chunk)[:3]
                # Add those 3 values into the list
                result.extend(values)

            return result  # We cut off the intensity value in the end, aint letting people mess with that.


@endpoint('razer.device.lighting.kraken', 'setCustom', in_sig='ai')
def set_custom_kraken(self, rgbi):
    """
    Set custom colour on kraken

    :return: List of args used for breathing effect
    :rtype: int
    """
    self.logger.debug("DBus call set custom")

    driver_path = self.get_driver_path('matrix_effect_custom')

    if len(rgbi) not in (3, 4):
        raise ValueError("List must be of 3 or 4 bytes")

    #DodgyCoding
    rgbi_list = list(rgbi)
    for index, item in enumerate(list(rgbi)):
        item = int(item)

        if item < 0:
            rgbi_list[index] = 0
        elif item > 255:
            rgbi_list[index] = 255
        else:
            rgbi_list[index] = item

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes(rgbi_list))


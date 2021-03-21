"""
Module for kraken methods
"""
from openrazer_daemon.dbus_services import endpoint


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

    # DodgyCoding
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

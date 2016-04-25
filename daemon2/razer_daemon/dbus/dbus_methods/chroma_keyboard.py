from razer_daemon.dbus import endpoint

@endpoint('device.misc', out_sig='d')
def get_brightness(self):
    """
    Get brightness

    :return: Brightness
    :rtype: float
    """
    driver_path = self.get_driver_path('set_brightness')

    with open(driver_path, 'r') as driver_file:
        brightness = float(driver_file.read()) * (100.0/255.0)
        return brightness

@endpoint('device.misc', in_sig='d')
def set_brightness(self, brightness):
    """
    Set brightness

    :param brightness: Brightness
    :type brightness: int
    """
    driver_path = self.get_driver_path('set_brightness')

    brightness = int(round(brightness * (255.0/100.0)))
    if brightness > 255:
        brightness = 255
    elif brightness < 0:
        brightness = 0

    with open(driver_path, 'w') as driver_file:
        driver_file.write(brightness)

from openrazer_daemon.dbus_services import endpoint


def _get_channel_brightness(self, channel):
    driver_path = self.get_driver_path(channel + '_led_brightness')
    with open(driver_path, 'r') as driver_file:
        return float(driver_file.read().strip()) / (255.0 / 100.0)


@endpoint('razer.device.lighting.channel1', 'getChannel1Brightness', out_sig='d')
def get_channel1_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_channel1_brightness")
    return _get_channel_brightness(self, "channel1")


@endpoint('razer.device.lighting.channel2', 'getChannel2Brightness', out_sig='d')
def get_channel2_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_channel2_brightness")
    return _get_channel_brightness(self, "channel2")


@endpoint('razer.device.lighting.channel3', 'getChannel3Brightness', out_sig='d')
def get_channel3_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_channel3_brightness")
    return _get_channel_brightness(self, "channel3")


@endpoint('razer.device.lighting.channel4', 'getChannel4Brightness', out_sig='d')
def get_channel4_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_channel4_brightness")
    return _get_channel_brightness(self, "channel4")


@endpoint('razer.device.lighting.channel5', 'getChannel5Brightness', out_sig='d')
def get_channel5_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_channel5_brightness")
    return _get_channel_brightness(self, "channel5")


@endpoint('razer.device.lighting.channel6', 'getChannel6Brightness', out_sig='d')
def get_channel6_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_channel6_brightness")
    return _get_channel_brightness(self, "channel6")


def _set_channel_brightness(self, channel, brightness):
    driver_path = self.get_driver_path(channel + '_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence(channel, "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.channel1', 'setChannel1Brightness', in_sig='d')
def set_channel1_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_channel1_brightness")
    _set_channel_brightness(self, "channel1", brightness)


@endpoint('razer.device.lighting.channel2', 'setChannel2Brightness', in_sig='d')
def set_channel2_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_channel2_brightness")
    _set_channel_brightness(self, "channel2", brightness)


@endpoint('razer.device.lighting.channel3', 'setChannel3Brightness', in_sig='d')
def set_channel3_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_channel3_brightness")
    _set_channel_brightness(self, "channel3", brightness)


@endpoint('razer.device.lighting.channel4', 'setChannel4Brightness', in_sig='d')
def set_channel4_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_channel4_brightness")
    _set_channel_brightness(self, "channel4", brightness)


@endpoint('razer.device.lighting.channel5', 'setChannel5Brightness', in_sig='d')
def set_channel5_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_channel5_brightness")
    _set_channel_brightness(self, "channel5", brightness)


@endpoint('razer.device.lighting.channel6', 'setChannel6Brightness', in_sig='d')
def set_channel6_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_channel6_brightness")
    _set_channel_brightness(self, "channel6", brightness)


def _get_channel_size(self, channel):
    driver_path = self.get_driver_path(channel + '_size')
    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


@endpoint('razer.device.lighting.channel1', 'getChannel1Size', out_sig='i')
def get_channel1_size(self):
    """
    Get the device's size
    :return: Size
    :rtype: float
    """
    self.logger.debug("DBus call get_channel1_size")
    return _get_channel_size(self, "channel1")


@endpoint('razer.device.lighting.channel2', 'getChannel2Size', out_sig='i')
def get_channel2_size(self):
    """
    Get the device's size
    :return: Size
    :rtype: float
    """
    self.logger.debug("DBus call get_channel2_size")
    return _get_channel_size(self, "channel2")


@endpoint('razer.device.lighting.channel3', 'getChannel3Size', out_sig='i')
def get_channel3_size(self):
    """
    Get the device's size
    :return: Size
    :rtype: float
    """
    self.logger.debug("DBus call get_channel3_size")
    return _get_channel_size(self, "channel3")


@endpoint('razer.device.lighting.channel4', 'getChannel4Size', out_sig='i')
def get_channel4_size(self):
    """
    Get the device's size
    :return: Size
    :rtype: float
    """
    self.logger.debug("DBus call get_channel4_size")
    return _get_channel_size(self, "channel4")


@endpoint('razer.device.lighting.channel5', 'getChannel5Size', out_sig='i')
def get_channel5_size(self):
    """
    Get the device's size
    :return: Size
    :rtype: float
    """
    self.logger.debug("DBus call get_channel5_size")
    return _get_channel_size(self, "channel5")


@endpoint('razer.device.lighting.channel6', 'getChannel6Size', out_sig='i')
def get_channel6_size(self):
    """
    Get the device's size
    :return: Size
    :rtype: float
    """
    self.logger.debug("DBus call get_channel6_size")
    return _get_channel_size(self, "channel6")


def _set_channel_size(self, channel, size):
    driver_path = self.get_driver_path(channel + '_size')

    self.method_args['size'] = size

    if size > 255:
        size = 255
    elif size < 0:
        size = 0

    self.set_persistence(channel, "size", int(size))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(size))

    # Notify others
    self.send_effect_event('setSize', size)


@endpoint('razer.device.lighting.channel1', 'setChannel1Size', in_sig='i')
def set_channel1_size(self, size):
    """
    Set the device's size
    :param size: Size
    :type size: int
    """
    self.logger.debug("DBus call set_channel1_size")
    _set_channel_size(self, "channel1", size)


@endpoint('razer.device.lighting.channel2', 'setChannel2Size', in_sig='i')
def set_channel2_size(self, size):
    """
    Set the device's size
    :param size: Size
    :type size: int
    """
    self.logger.debug("DBus call set_channel2_size")
    _set_channel_size(self, "channel2", size)


@endpoint('razer.device.lighting.channel3', 'setChannel3Size', in_sig='i')
def set_channel3_size(self, size):
    """
    Set the device's size
    :param size: Size
    :type size: int
    """
    self.logger.debug("DBus call set_channel3_size")
    _set_channel_size(self, "channel3", size)


@endpoint('razer.device.lighting.channel4', 'setChannel4Size', in_sig='i')
def set_channel4_size(self, size):
    """
    Set the device's size
    :param size: Size
    :type size: int
    """
    self.logger.debug("DBus call set_channel4_size")
    _set_channel_size(self, "channel4", size)


@endpoint('razer.device.lighting.channel5', 'setChannel5Size', in_sig='i')
def set_channel5_size(self, size):
    """
    Set the device's size
    :param size: Size
    :type size: int
    """
    self.logger.debug("DBus call set_channel5_size")
    _set_channel_size(self, "channel5", size)


@endpoint('razer.device.lighting.channel6', 'setChannel6Size', in_sig='i')
def set_channel6_size(self, size):
    """
    Set the device's size
    :param size: Size
    :type size: int
    """
    self.logger.debug("DBus call set_channel6_size")
    _set_channel_size(self, "channel6", size)

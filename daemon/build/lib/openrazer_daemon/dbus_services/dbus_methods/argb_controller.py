# SPDX-License-Identifier: GPL-2.0-or-later

from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.channel', 'getNumChannels', out_sig='q')
def get_num_channels(self):
    return self.NUM_CHANNELS


def _get_channel_brightness(self, channel):
    driver_path = self.get_driver_path(channel + '_led_brightness')
    with open(driver_path, 'r') as driver_file:
        return float(driver_file.read().strip()) / (255.0 / 100.0)


@endpoint('razer.device.lighting.channel', 'getChannelBrightness', in_sig='q', out_sig='d')
def get_channel_brightness(self, channel):
    """
    Get the channel brightness

    :param channel: Channel number to get the brightness of
    :type channel: int

    :return: Brightness
    :rtype: float
    """
    channel_name = 'channel{}'.format(channel)
    self.logger.debug("DBus call get_{}_brightness".format(channel_name))
    return _get_channel_brightness(self, channel_name)


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


@endpoint('razer.device.lighting.channel', 'setChannelBrightness', in_sig='qd')
def set_channel_brightness(self, channel, brightness):
    """
    Set the device's brightness

    :param channel: Channel
    :type channel: int

    :param brightness: Brightness
    :type brightness: int
    """
    channel_name = 'channel{}'.format(channel)
    self.logger.debug("DBus call set_{}_brightness".format(channel_name))
    _set_channel_brightness(self, channel_name, brightness)


def _get_channel_size(self, channel):
    driver_path = self.get_driver_path(channel + '_size')
    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


@endpoint('razer.device.lighting.channel', 'getChannelSize', in_sig='q', out_sig='i')
def get_channel_size(self, channel):
    """
    Get the device's size

    :param channel: Channel
    :type channel: int

    :return: Size
    :rtype: float
    """
    channel_name = 'channel{}'.format(channel)
    self.logger.debug("DBus call get_{}_size".format(channel_name))
    return _get_channel_size(self, channel_name)


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


@endpoint('razer.device.lighting.channel', 'setChannelSize', in_sig='qi')
def set_channel_size(self, channel, size):
    """
    Set the device's size
    :param channel: Channel
    :type channel: int

    :param size: Size
    :type size: int
    """
    channel_name = 'channel{}'.format(channel)

    self.logger.debug("DBus call set_{}_size".format(channel_name))
    _set_channel_size(self, channel_name, size)

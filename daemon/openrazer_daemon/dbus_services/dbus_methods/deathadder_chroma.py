# SPDX-License-Identifier: GPL-2.0-or-later

from openrazer_daemon.dbus_services import endpoint


def set_led_effect_common(self, zone: str, effect: str) -> None:
    driver_path = self.get_driver_path(zone + '_led_effect')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(effect)


@endpoint('razer.device.lighting.backlight', 'getBacklightActive', out_sig='b')
def get_backlight_active(self):
    """
    Get if the backlight is lit up
    """
    self.logger.debug("DBus call get_backlight_active")

    return self.zone["backlight"]["active"]


@endpoint('razer.device.lighting.backlight', 'setBacklightActive', in_sig='b')
def set_backlight_active(self, active):
    """
    Get if the backlight is lit up
    """
    self.logger.debug("DBus call set_backlight_active")

    # remember status
    self.set_persistence("backlight", "active", bool(active))

    driver_path = self.get_driver_path('backlight_led_state')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1' if active else '0')


@endpoint('razer.device.lighting.backlight', 'getBacklightBrightness', out_sig='d')
def get_backlight_brightness(self):
    """
    Get the device's brightness
    """
    self.logger.debug("DBus call get_backlight_brightness")

    return self.zone["backlight"]["brightness"]


@endpoint('razer.device.lighting.backlight', 'setBacklightBrightness', in_sig='d')
def set_backlight_brightness(self, brightness):
    """
    Set the device's brightness
    """
    self.logger.debug("DBus call set_backlight_brightness")

    driver_path = self.get_driver_path('backlight_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("backlight", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.logo', 'getLogoActive', out_sig='b')
def get_logo_active(self):
    """
    Get if the logo is lit up
    """
    self.logger.debug("DBus call get_logo_active")

    return self.zone["logo"]["active"]


@endpoint('razer.device.lighting.logo', 'setLogoActive', in_sig='b')
def set_logo_active(self, active):
    """
    Set if the logo is lit up
    """
    self.logger.debug("DBus call set_logo_active")

    # remember status
    self.set_persistence("logo", "active", bool(active))

    driver_path = self.get_driver_path('logo_led_state')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1' if active else '0')


@endpoint('razer.device.lighting.logo', 'getLogoBrightness', out_sig='d')
def get_logo_brightness(self):
    """
    Get the device's brightness
    """
    self.logger.debug("DBus call get_logo_brightness")

    return self.zone["logo"]["brightness"]


@endpoint('razer.device.lighting.logo', 'setLogoBrightness', in_sig='d')
def set_logo_brightness(self, brightness):
    """
    Set the device's brightness
    """
    self.logger.debug("DBus call set_logo_brightness")

    driver_path = self.get_driver_path('logo_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("logo", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.logo', 'setLogoStaticMono')
def set_logo_static_mono(self):
    """
    Set the device to static colour
    """
    self.logger.debug("DBus call set_logo_static_mono")

    # Notify others
    self.send_effect_event('setStatic')

    set_led_effect_common(self, 'logo', '0')


@endpoint('razer.device.lighting.logo', 'setLogoPulsateMono')
def set_logo_pulsate_mono(self):
    """
    Set the device to pulsate
    """
    self.logger.debug("DBus call set_logo_pulsate_mono")

    # Notify others
    self.send_effect_event('setPulsate')

    set_led_effect_common(self, 'logo', '2')


@endpoint('razer.device.lighting.scroll', 'getScrollActive', out_sig='b')
def get_scroll_active(self):
    """
    Get if the scroll is light up
    """
    self.logger.debug("DBus call get_scroll_active")

    return self.zone["scroll"]["active"]


@endpoint('razer.device.lighting.scroll', 'setScrollActive', in_sig='b')
def set_scroll_active(self, active):
    """
    Get if the scroll is light up
    """
    self.logger.debug("DBus call set_scroll_active")

    # remember status
    self.set_persistence("scroll", "active", bool(active))

    driver_path = self.get_driver_path('scroll_led_state')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1' if active else '0')


@endpoint('razer.device.lighting.scroll', 'getScrollBrightness', out_sig='d')
def get_scroll_brightness(self):
    """
    Get the device's brightness
    """
    self.logger.debug("DBus call get_scroll_brightness")

    return self.zone["scroll"]["brightness"]


@endpoint('razer.device.lighting.scroll', 'setScrollBrightness', in_sig='d')
def set_scroll_brightness(self, brightness):
    """
    Set the device's brightness
    """
    self.logger.debug("DBus call set_scroll_brightness")

    driver_path = self.get_driver_path('scroll_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("scroll", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.scroll', 'setScrollStaticMono')
def set_scroll_static_mono(self):
    """
    Set the device to static colour
    """
    self.logger.debug("DBus call set_scroll_static_mono")

    # Notify others
    self.send_effect_event('setStatic')

    set_led_effect_common(self, 'scroll', '0')


@endpoint('razer.device.lighting.scroll', 'setScrollPulsateMono')
def set_scroll_pulsate_mono(self):
    """
    Set the device to pulsate
    """
    self.logger.debug("DBus call set_scroll_pulsate_mono")

    # Notify others
    self.send_effect_event('setPulsate')

    set_led_effect_common(self, 'scroll', '2')

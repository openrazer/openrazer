from openrazer_daemon.dbus_services import endpoint


def set_led_effect_color_common(self, zone: str, effect: str, red: int, green: int, blue: int) -> None:
    rgb_driver_path = self.get_driver_path(zone + '_led_rgb')
    effect_driver_path = self.get_driver_path(zone + '_led_effect')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file, open(effect_driver_path, 'w') as effect_driver_file:
        rgb_driver_file.write(payload)
        effect_driver_file.write(effect)


def set_led_effect_common(self, zone: str, effect: str) -> None:
    driver_path = self.get_driver_path(zone + '_led_effect')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(effect)


@endpoint('razer.device.lighting.backlight', 'getBacklightActive', out_sig='b')
def get_backlight_active(self):
    """
    Get if the backlight is lit up

    :return: Active
    :rtype: bool
    """
    self.logger.debug("DBus call get_backlight_active")

    return self.zone["backlight"]["active"]


@endpoint('razer.device.lighting.backlight', 'setBacklightActive', in_sig='b')
def set_backlight_active(self, active):
    """
    Get if the backlight is lit up

    :param active: Is active
    :type active: bool
    """
    self.logger.debug("DBus call set_backlight_active")

    # remember status
    self.set_persistence("backlight", "active", bool(active))

    driver_path = self.get_driver_path('backlight_led_state')

    with open(driver_path, 'w') as driver_file:
        if active:
            driver_file.write('1')
        else:
            driver_file.write('0')


@endpoint('razer.device.lighting.logo', 'getLogoActive', out_sig='b')
def get_logo_active(self):
    """
    Get if the logo is lit up

    :return: Active
    :rtype: bool
    """
    self.logger.debug("DBus call get_logo_active")

    return self.zone["logo"]["active"]


@endpoint('razer.device.lighting.logo', 'setLogoActive', in_sig='b')
def set_logo_active(self, active):
    """
    Set if the logo is lit up

    :param active: Is active
    :type active: bool
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

    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_logo_brightness")

    return self.zone["logo"]["brightness"]


@endpoint('razer.device.lighting.logo', 'setLogoBrightness', in_sig='d')
def set_logo_brightness(self, brightness):
    """
    Set the device's brightness

    :param brightness: Brightness
    :type brightness: int
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


@endpoint('razer.device.lighting.logo', 'setLogoStatic', in_sig='yyy')
def set_logo_static(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_static_effect")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    # remember effect
    self.set_persistence("logo", "effect", 'static')
    self.zone["logo"]["colors"][0:3] = int(red), int(green), int(blue)

    set_led_effect_color_common(self, 'logo', '0', red, green, blue)


@endpoint('razer.device.lighting.logo', 'setLogoStaticMono')
def set_logo_static_mono(self):
    """
    Set the device to static colour
    """
    self.logger.debug("DBus call set_logo_static_mono")

    # Notify others
    self.send_effect_event('setStatic')

    set_led_effect_common(self, 'logo', '0')


@endpoint('razer.device.lighting.logo', 'setLogoBlinking', in_sig='yyy')
def set_logo_blinking(self, red, green, blue):
    """
    Set the device to blinking

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_logo_blinking")

    # Notify others
    self.send_effect_event('setLogoBlinking', red, green, blue)

    # remember effect
    self.set_persistence("logo", "effect", 'blinking')
    self.zone["logo"]["colors"][0:3] = int(red), int(green), int(blue)

    set_led_effect_color_common(self, 'logo', '1', red, green, blue)


@endpoint('razer.device.lighting.logo', 'setLogoPulsate', in_sig='yyy')
def set_logo_pulsate(self, red, green, blue):
    """
    Set the device to pulsate

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_logo_pulsing")

    # Notify others
    self.send_effect_event('setPulsate', red, green, blue)

    # remember effect
    self.set_persistence("logo", "effect", 'pulsate')
    self.zone["logo"]["colors"][0:3] = int(red), int(green), int(blue)

    set_led_effect_color_common(self, 'logo', '2', red, green, blue)


@endpoint('razer.device.lighting.logo', 'setLogoPulsateMono')
def set_logo_pulsate_mono(self):
    """
    Set the device to pulsate
    """
    self.logger.debug("DBus call set_logo_pulsate_mono")

    # Notify others
    self.send_effect_event('setPulsate')

    set_led_effect_common(self, 'logo', '2')


@endpoint('razer.device.lighting.logo', 'setLogoSpectrum')
def set_logo_spectrum(self):
    """
    Set the device to spectrum

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_logo_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("logo", "effect", 'spectrum')

    set_led_effect_common(self, 'logo', '4')


@endpoint('razer.device.lighting.scroll', 'getScrollActive', out_sig='b')
def get_scroll_active(self):
    """
    Get if the scroll is light up

    :return: Active
    :rtype: bool
    """
    self.logger.debug("DBus call get_scroll_active")

    return self.zone["scroll"]["active"]


@endpoint('razer.device.lighting.scroll', 'setScrollActive', in_sig='b')
def set_scroll_active(self, active):
    """
    Get if the scroll is light up

    :param active: Is active
    :type active: bool
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

    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_scroll_brightness")

    return self.zone["scroll"]["brightness"]


@endpoint('razer.device.lighting.scroll', 'setScrollBrightness', in_sig='d')
def set_scroll_brightness(self, brightness):
    """
    Set the device's brightness

    :param brightness: Brightness
    :type brightness: int
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


@endpoint('razer.device.lighting.scroll', 'setScrollStatic', in_sig='yyy')
def set_scroll_static(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_scroll_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    # remember effect
    self.set_persistence("scroll", "effect", 'static')
    self.zone["scroll"]["colors"][0:3] = int(red), int(green), int(blue)

    set_led_effect_color_common(self, 'scroll', '0', red, green, blue)


@endpoint('razer.device.lighting.scroll', 'setScrollStaticMono')
def set_scroll_static_mono(self):
    """
    Set the device to static colour
    """
    self.logger.debug("DBus call set_scroll_static_mono")

    # Notify others
    self.send_effect_event('setStatic')

    set_led_effect_common(self, 'scroll', '0')


@endpoint('razer.device.lighting.scroll', 'setScrollBlinking', in_sig='yyy')
def set_scroll_blinking(self, red, green, blue):
    """
    Set the device to blinking

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_scroll_pulsate")

    # Notify others
    self.send_effect_event('setPulsate', red, green, blue)

    # remember effect
    self.set_persistence("scroll", "effect", 'blinking')
    self.zone["scroll"]["colors"][0:3] = int(red), int(green), int(blue)

    set_led_effect_color_common(self, 'scroll', '1', red, green, blue)


@endpoint('razer.device.lighting.scroll', 'setScrollPulsate', in_sig='yyy')
def set_scroll_pulsate(self, red, green, blue):
    """
    Set the device to pulsate

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_scroll_breathing")

    # Notify others
    self.send_effect_event('setPulsate', red, green, blue)

    # remember effect
    self.set_persistence("scroll", "effect", 'pulsate')
    self.zone["scroll"]["colors"][0:3] = int(red), int(green), int(blue)

    set_led_effect_color_common(self, 'scroll', '2', red, green, blue)


@endpoint('razer.device.lighting.scroll', 'setScrollPulsateMono')
def set_scroll_pulsate_mono(self):
    """
    Set the device to pulsate
    """
    self.logger.debug("DBus call set_scroll_pulsate_mono")

    # Notify others
    self.send_effect_event('setPulsate')

    set_led_effect_common(self, 'scroll', '2')


@endpoint('razer.device.lighting.scroll', 'setScrollSpectrum')
def set_scroll_spectrum(self):
    """
    Set the device to spectrum
    """
    self.logger.debug("DBus call set_scroll_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("scroll", "effect", 'spectrum')

    set_led_effect_common(self, 'scroll', '4')

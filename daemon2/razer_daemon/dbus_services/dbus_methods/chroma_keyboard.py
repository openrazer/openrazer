"""
BlackWidow Chroma Effects
"""
from razer_daemon.dbus_services import endpoint

@endpoint('razer.device.misc', 'getBrightness', out_sig='d')
def get_brightness(self):
    """
    Get the device's brightness

    :return: Brightness
    :rtype: float
    """
    driver_path = self.get_driver_path('set_brightness')

    with open(driver_path, 'r') as driver_file:
        brightness = float(driver_file.read()) * (100.0/255.0)
        return round(brightness, 2)

@endpoint('razer.device.misc', 'setBrightness', in_sig='d')
def set_brightness(self, brightness):
    """
    Set the device's brightness

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
        driver_file.write(str(brightness))

@endpoint('razer.device.misc', 'enableMacroKeys')
def enable_macro_keys(self):
    """
    Make macro keys return keycodes
    """
    driver_path = self.get_driver_path('macro_keys')

    with open(driver_path, 'w') as driver_file:
        return driver_file.write('1')

@endpoint('razer.device.misc', 'getGameMode', out_sig='b')
def get_game_mode(self):
    """
    Get game mode LED state

    :return: Game mode LED state
    :rtype: bool
    """
    driver_path = self.get_driver_path('mode_game')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip() == '1'

@endpoint('razer.device.misc', 'setGameMode', in_sig='b')
def set_game_mode(self, enable):
    """
    Set game mode LED state

    :param enable: Status of game mode
    :type enable: bool
    """
    driver_path = self.get_driver_path('mode_game')

    with open(driver_path, 'w') as driver_file:
        if enable:
            driver_file.write('1')
        else:
            driver_file.write('0')

@endpoint('razer.device.misc', 'getMacroMode', out_sig='b')
def get_macro_mode(self):
    """
    Get macro mode LED state

    :return: Status of macro mode
    :rtype: bool
    """
    driver_path = self.get_driver_path('mode_macro')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip() == '1'

@endpoint('razer.device.misc', 'setMacroMode', in_sig='b')
def set_macro_mode(self, enable):
    """
    Set macro mode LED state

    :param enable: Status of macro mode
    :type enable: bool
    """
    driver_path = self.get_driver_path('mode_macro')

    with open(driver_path, 'w') as driver_file:
        if enable:
            driver_file.write('1')
        else:
            driver_file.write('0')

@endpoint('razer.device.misc', 'getMacroEffect', out_sig='i')
def get_macro_effect(self):
    """
    Get the effect on the macro LED

    :return: Macro LED effect ID
    :rtype: int
    """
    driver_path = self.get_driver_path('mode_macro_effect')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())

@endpoint('razer.device.misc', 'setMacroEffect', in_sig='y')
def set_macro_effect(self, effect):
    """
    Set the effect on the macro LED

    :param effect: Macro LED effect ID
    :type effect: int
    """
    driver_path = self.get_driver_path('mode_macro_effect')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(effect)))


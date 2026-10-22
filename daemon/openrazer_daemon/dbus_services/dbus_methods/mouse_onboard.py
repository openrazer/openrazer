# SPDX-License-Identifier: GPL-2.0-or-later

from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.onboard', 'getSidePlate', out_sig='y')
def get_side_plate(self):
    """
    Get the attached side plate

    :return: 0 = none, 1 = 2-button, 3 = 12-button, 4 = 6-button
    :rtype: int
    """
    self.logger.debug("DBus call get_side_plate")

    driver_path = self.get_driver_path('side_plate')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


def _check_button(store, button, layer):
    if store not in range(1, 6):
        raise ValueError("store has to be between 1 and 5")
    if button not in range(1, 0x100):
        raise ValueError("button has to be between 1 and 255")
    if layer not in (0, 1):
        raise ValueError("layer has to be 0 (standard) or 1 (Hypershift)")


@endpoint('razer.device.onboard', 'getButtons', out_sig='ay')
def get_onboard_buttons(self):
    """
    Get every button id the device has, across all side plates

    :return: Button ids
    :rtype: bytes
    """
    self.logger.debug("DBus call get_onboard_buttons")

    return list(self.ONBOARD_BUTTONS)


@endpoint('razer.device.onboard', 'getButtonAction', in_sig='yyy', out_sig='ay')
def get_button_action(self, store, button, layer):
    """
    Get the action stored on the device for a button

    :param store: 1 = the live profile, 2-5 = onboard profiles
    :type store: int

    :param button: Button id
    :type button: int

    :param layer: 0 = standard, 1 = Hypershift
    :type layer: int

    :return: <type> <length> <data...>. Factory entries can report a length of 1 with more data bytes.
    :rtype: bytes
    """
    self.logger.debug("DBus call get_button_action")

    _check_button(store, button, layer)

    return list(_read_button_action(self, store, button, layer))


def _read_button_action(self, store, button, layer):
    driver_path = self.get_driver_path('button_action')

    # Another sysfs writer can change the selected button between the write and the read
    for _ in range(3):
        with open(driver_path, 'wb') as driver_file:
            driver_file.write(bytes((store, button, layer)))
        with open(driver_path, 'rb') as driver_file:
            reply = driver_file.read().ljust(10, b'\0')
        if reply[0] == store and reply[1] == button:
            break
    else:
        raise OSError("button_action kept returning another button")

    end = max(5 + reply[4], len(reply.rstrip(b'\0')))
    return reply[3:end]


@endpoint('razer.device.onboard', 'setButtonAction', in_sig='yyyay', byte_arrays=True)
def set_button_action(self, store, button, layer, action):
    """
    Store an action on the device for a button

    :param store: 1 = the live profile, 2-5 = onboard profiles
    :type store: int

    :param button: Button id
    :type button: int

    :param layer: 0 = standard, 1 = Hypershift
    :type layer: int

    :param action: <type> <length> <data...>, 2 to 7 bytes
    :type action: bytes
    """
    self.logger.debug("DBus call set_button_action")

    _check_button(store, button, layer)
    if len(action) not in range(2, 8):
        raise ValueError("action has to be between 2 and 7 bytes")

    driver_path = self.get_driver_path('button_action')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes((store, button, layer)) + bytes(action))


@endpoint('razer.device.onboard', 'addMacro', in_sig='qay', byte_arrays=True)
def add_onboard_macro(self, macro_id, events):
    """
    Upload a macro to the device's memory

    Deleted macros keep their space until the device reclaims it, so prefer reusing ids.

    :param macro_id: Macro id, 1 to 65535, not yet in use
    :type macro_id: int

    :param events: Event stream: 0x01 <usage> key down, 0x02 <usage> key up, 0x11 <ms> and 0x12 <ms u16 BE> delays
    :type events: bytes
    """
    self.logger.debug("DBus call add_onboard_macro")

    if macro_id not in range(1, 0x10000):
        raise ValueError("macro_id has to be between 1 and 65535")
    if len(events) not in range(1, 4095):
        raise ValueError("events have to be between 1 and 4094 bytes")

    driver_path = self.get_driver_path('macro')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(macro_id.to_bytes(2, 'big') + bytes(events))


@endpoint('razer.device.onboard', 'deleteMacro', in_sig='q')
def delete_onboard_macro(self, macro_id):
    """
    Delete a macro from the device's memory

    Buttons bound to the macro keep the binding; the client library checks for them first.

    :param macro_id: Macro id
    :type macro_id: int
    """
    self.logger.debug("DBus call delete_onboard_macro")

    driver_path = self.get_driver_path('macro_delete')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(macro_id)))


@endpoint('razer.device.onboard', 'getMacros', out_sig='aq')
def get_onboard_macros(self):
    """
    Get the ids of the macros in the device's memory

    After a delete the device can leave live ids out of this list.

    :return: Macro ids
    :rtype: list of int
    """
    self.logger.debug("DBus call get_onboard_macros")

    driver_path = self.get_driver_path('macro_list')

    with open(driver_path, 'rb') as driver_file:
        data = driver_file.read()

    return [int.from_bytes(data[i:i + 2], 'big') for i in range(0, len(data) - 1, 2)]


@endpoint('razer.device.onboard', 'getMacroStorage', out_sig='au')
def get_onboard_macro_storage(self):
    """
    Get the device's macro memory usage

    :return: [max macros, total bytes, free bytes, bytes of deleted macros not yet reclaimed]
    :rtype: list of int
    """
    self.logger.debug("DBus call get_onboard_macro_storage")

    driver_path = self.get_driver_path('macro_storage')

    with open(driver_path, 'r') as driver_file:
        return [int(value) for value in driver_file.read().split()]

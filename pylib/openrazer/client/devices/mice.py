# SPDX-License-Identifier: GPL-2.0-or-later

import dbus as _dbus  # type: ignore

from openrazer.client.devices import RazerDevice as __RazerDevice


class RazerMouse(__RazerDevice):
    @property
    def max_dpi(self) -> int:
        """
        Gets max DPI

        :return: Max DPI, if device does not have DPI it'll raise an exception
        :rtype: int
        """
        if self.has('dpi'):
            return int(self._dbus_interfaces['dpi'].maxDPI())
        else:
            raise NotImplementedError()

    @property
    def available_dpi(self) -> list[int]:
        """
        Gets the available DPI

        :return: Available DPI, if device has only a couple of fixed possible DPI values
        :rtype: list
        """
        if self.has('available_dpi'):
            dbuslist = self._dbus_interfaces['dpi'].availableDPI()
            # Repack list from dbus ints to normal ints
            return [int(d) for d in dbuslist]
        else:
            raise NotImplementedError()

    @property
    def dpi(self) -> tuple[int, int]:
        """
        Get mouse DPI

        Will return a tuple
        :return: DPI (500, 500)
        :rtype: tuple

        :raises NotImplementedError: If function is not supported
        """
        if self.has('available_dpi'):
            dpi_x = self._dbus_interfaces['dpi'].getDPI()[0]
            return int(dpi_x), 0
        elif self.has('dpi'):
            dpi_x, dpi_y = self._dbus_interfaces['dpi'].getDPI()
            # Converting to integers to remove the dbus types
            return int(dpi_x), int(dpi_y)
        else:
            raise NotImplementedError()

    @dpi.setter
    def dpi(self, value: tuple[int, int]) -> None:
        """
        Set mouse dpi

        Daemon does type validation but can't be too careful
        :param value: DPI X, Y tuple
        :type value: tuple

        :raises ValueError: If the tuple isn't long enough or contains invalid crap
        :raises NotImplementedError: If function is not supported
        """
        if self.has('dpi'):
            if len(value) != 2:
                raise ValueError("DPI tuple is not of length 2. Length: {0}".format(len(value)))
            max_dpi = self.max_dpi
            dpi_x, dpi_y = value
            dpi_x_only = self.has('available_dpi')

            if not isinstance(dpi_x, int) or not isinstance(dpi_y, int):
                raise ValueError("DPI X or Y is not an integer, X:{0} Y:{1}".format(type(dpi_x), type(dpi_y)))

            if dpi_x < 0 or dpi_x > max_dpi:
                raise ValueError("DPI X either too small or too large, X:{0}".format(dpi_x))

            if dpi_x_only and not dpi_y == 0:
                raise ValueError("DPI Y is not supported for this device")
            elif dpi_y < 0 or dpi_y > max_dpi:
                raise ValueError("DPI Y either too small or too large, Y:{0}".format(dpi_y))

            self._dbus_interfaces['dpi'].setDPI(dpi_x, dpi_y)
        else:
            raise NotImplementedError()

    @property
    def dpi_stages(self) -> tuple[int, list[tuple[int, int]]]:
        """
        Get mouse DPI stages

        Will return a tuple containing the active DPI stage number and the list
        of DPI stages as tuples.
        The active DPI stage number must be: >= 1 and <= nr of DPI stages.
        :return: active DPI stage number and DPI stages
                 (1, [(500, 500), (1000, 1000), (2000, 2000) ...]
        :rtype: (int, list)

        :raises NotImplementedError: if function is not supported
        """
        if self.has('dpi_stages'):
            response = self._dbus_interfaces['dpi'].getDPIStages()
            dpi_stages = []

            active_stage = int(response[0])

            for dpi_x, dpi_y in response[1]:
                dpi_stages.append((int(dpi_x), int(dpi_y)))

            return (active_stage, dpi_stages)
        else:
            raise NotImplementedError()

    @dpi_stages.setter
    def dpi_stages(self, value: tuple[int, list[tuple[int, int]]]) -> None:
        """
        Set mouse DPI stages

        Daemon does type validation but can't be too careful
        :param value: active DPI stage number and list of DPI X, Y tuples
        :type value: (int, list)

        :raises ValueError: when the input is invalid
        :raises NotImplementedError: If function is not supported
        """
        if self.has('dpi_stages'):
            max_dpi = self.max_dpi
            dpi_stages = []

            active_stage = value[0]
            if not isinstance(active_stage, int):
                raise ValueError(
                    "Active DPI stage is not an integer: {0}".format(
                        type(active_stage)))

            if active_stage < 1:
                raise ValueError(
                    "Active DPI stage has invalid value: {0} < 1".format(
                        active_stage))

            for stage in value[1]:
                if len(stage) != 2:
                    raise ValueError(
                        "DPI tuple is not of length 2. Length: {0}".format(
                            len(stage)))

                dpi_x, dpi_y = stage

                if not isinstance(dpi_x, int) or not isinstance(dpi_y, int):
                    raise ValueError(
                        "DPI X or Y is not an integer, X:{0} Y:{1}".format(
                            type(dpi_x), type(dpi_y)))

                if dpi_x < 0 or dpi_x > max_dpi:
                    raise ValueError(
                        "DPI X either too small or too large, X:{0}".format(
                            dpi_x))
                if dpi_y < 0 or dpi_y > max_dpi:
                    raise ValueError(
                        "DPI Y either too small or too large, Y:{0}".format(
                            dpi_y))

                dpi_stages.append((dpi_x, dpi_y))

            if active_stage > len(dpi_stages):
                raise ValueError(
                    "Active DPI stage has invalid value: {0} > {1}".format(
                        active_stage, len(dpi_stages)))

            self._dbus_interfaces['dpi'].setDPIStages(active_stage, dpi_stages)
        else:
            raise NotImplementedError()

    @property
    def scroll_mode(self) -> int:
        """
        Get the scroll wheel mode of the device

        :return: The device's current scroll mode (0 = tactile, 1 = free spin)
        :rtype: int

        :raises NotImplementedError: If function is not supported
        """
        if self.has('scroll_mode'):
            return int(self._dbus_interfaces['scroll'].getScrollMode())
        else:
            raise NotImplementedError()

    @scroll_mode.setter
    def scroll_mode(self, mode: int) -> None:
        """
        Set the scroll mode of the device

        :param mode: The mode to set (0 = tactile, 1 = free spin)
        :type mode: int

        :raises NotImplementedError: If function is not supported
        """
        if self.has('scroll_mode'):
            self._dbus_interfaces['scroll'].setScrollMode(mode)
        else:
            raise NotImplementedError()

    @property
    def scroll_acceleration(self) -> bool:
        """
        Get the device's scroll acceleration state

        :return: true if acceleration enabled, false otherwise
        :rtype: bool

        :raises NotImplementedError: If function is not supported
        """
        if self.has('scroll_acceleration'):
            return bool(self._dbus_interfaces['scroll'].getScrollAcceleration())
        else:
            raise NotImplementedError()

    @scroll_acceleration.setter
    def scroll_acceleration(self, enabled: bool) -> None:
        """
        Set the device's scroll acceleration state

        :param enabled: true to enable acceleration, false to disable it
        :type enabled: bool

        :raises NotImplementedError: If function is not supported
        """
        if self.has('scroll_acceleration'):
            self._dbus_interfaces['scroll'].setScrollAcceleration(enabled)
        else:
            raise NotImplementedError()

    @property
    def scroll_smart_reel(self) -> bool:
        """
        Get the device's "smart reel" state

        :return: true if smart reel enabled, false otherwise
        :rtype: bool

        :raises NotImplementedError: If function is not supported
        """
        if self.has('scroll_smart_reel'):
            return bool(self._dbus_interfaces['scroll'].getScrollSmartReel())
        else:
            raise NotImplementedError()

    @scroll_smart_reel.setter
    def scroll_smart_reel(self, enabled: bool) -> None:
        """
        Set the device's "smart reel" state

        :param enabled: true to enable smart reel, false to disable it
        :type enabled: bool

        :raises NotImplementedError: If function is not supported
        """
        if self.has('scroll_smart_reel'):
            self._dbus_interfaces['scroll'].setScrollSmartReel(enabled)
        else:
            raise NotImplementedError()

    @property
    def side_plate(self) -> int:
        """
        Get the attached side plate

        :return: 0 = none, 1 = 2-button, 3 = 12-button, 4 = 6-button
        :rtype: int

        :raises NotImplementedError: If function is not supported
        """
        if self.has('onboard_buttons'):
            return int(self._dbus_interfaces['onboard'].getSidePlate())
        else:
            raise NotImplementedError()

    def get_button_action(self, button: int, layer: int = 0, store: int = 1) -> bytes:
        """
        Get the action stored on the device for a button

        :param button: Button id
        :type button: int

        :param layer: 0 = standard, 1 = Hypershift
        :type layer: int

        :param store: 1 = the live profile, 2-5 = onboard profiles
        :type store: int

        :return: <type> <length> <data...>
        :rtype: bytes

        :raises NotImplementedError: If function is not supported
        """
        if self.has('onboard_buttons'):
            return bytes(self._dbus_interfaces['onboard'].getButtonAction(store, button, layer))
        else:
            raise NotImplementedError()

    def set_button_action(self, button: int, action: bytes, layer: int = 0, stores: tuple[int, ...] = (1,)) -> None:
        """
        Store an action on the device for a button

        The device runs store 1 and keeps it across power cycles. Synapse also writes onboard profile 2.

        :param button: Button id
        :type button: int

        :param action: <type> <length> <data...>, 2 to 7 bytes
        :type action: bytes

        :param layer: 0 = standard, 1 = Hypershift
        :type layer: int

        :param stores: Stores to write, in order
        :type stores: tuple of int

        :raises NotImplementedError: If function is not supported
        """
        if self.has('onboard_buttons'):
            for store in stores:
                self._dbus_interfaces['onboard'].setButtonAction(store, button, layer, _dbus.ByteArray(action))
        else:
            raise NotImplementedError()

    def set_button_key(self, button: int, usage: int, modifiers: int = 0, layer: int = 0) -> None:
        """
        Make a button send a key

        :param button: Button id
        :type button: int

        :param usage: HID keyboard usage, e.g. 0x04 for A
        :type usage: int

        :param modifiers: HID modifier bitmask, e.g. 0x01 for left Ctrl
        :type modifiers: int

        :param layer: 0 = standard, 1 = Hypershift
        :type layer: int
        """
        self.set_button_action(button, bytes((0x02, 0x02, modifiers, usage)), layer)

    def set_button_mouse(self, button: int, mouse_button: int, layer: int = 0) -> None:
        """
        Make a button act as a mouse button

        :param button: Button id
        :type button: int

        :param mouse_button: 1 = left, 2 = right, 3 = middle, 4 = back, 5 = forward
        :type mouse_button: int

        :param layer: 0 = standard, 1 = Hypershift
        :type layer: int
        """
        self.set_button_action(button, bytes((0x01, 0x01, mouse_button)), layer)

    def set_button_macro(self, button: int, macro_id: int, layer: int = 0, while_held: bool = False) -> None:
        """
        Make a button play an onboard macro

        :param button: Button id
        :type button: int

        :param macro_id: Id of a macro added with add_onboard_macro
        :type macro_id: int

        :param layer: 0 = standard, 1 = Hypershift
        :type layer: int

        :param while_held: Repeat while the button is held instead of playing once
        :type while_held: bool
        """
        if while_held:
            action = bytes((0x04, 0x02)) + macro_id.to_bytes(2, 'big')
        else:
            action = bytes((0x03, 0x03)) + macro_id.to_bytes(2, 'big') + b'\x01'
        self.set_button_action(button, action, layer)

    @property
    def onboard_macros(self) -> list[int]:
        """
        Get the ids of the macros in the device's memory

        After a delete the device can leave live ids out of this list.

        :return: Macro ids
        :rtype: list of int

        :raises NotImplementedError: If function is not supported
        """
        if self.has('onboard_macros'):
            return [int(macro_id) for macro_id in self._dbus_interfaces['onboard'].getMacros()]
        else:
            raise NotImplementedError()

    @property
    def onboard_macro_storage(self) -> dict[str, int]:
        """
        Get the device's macro memory usage

        :return: max_macros, total, free and deleted (bytes of deleted macros not yet reclaimed)
        :rtype: dict

        :raises NotImplementedError: If function is not supported
        """
        if self.has('onboard_macros'):
            values = [int(value) for value in self._dbus_interfaces['onboard'].getMacroStorage()]
            return dict(zip(('max_macros', 'total', 'free', 'deleted'), values))
        else:
            raise NotImplementedError()

    def add_onboard_macro(self, macro_id: int, events: bytes) -> None:
        """
        Upload a macro to the device's memory

        Deleted macros keep their space until the device reclaims it, so prefer reusing ids.

        :param macro_id: Macro id, 1 to 65535, not yet in use
        :type macro_id: int

        :param events: 0x01 <usage> key down, 0x02 <usage> key up, 0x11 <ms> and 0x12 <ms u16 BE> delays
        :type events: bytes

        :raises NotImplementedError: If function is not supported
        """
        if self.has('onboard_macros'):
            self._dbus_interfaces['onboard'].addMacro(macro_id, _dbus.ByteArray(events))
        else:
            raise NotImplementedError()

    def delete_onboard_macro(self, macro_id: int) -> None:
        """
        Delete a macro from the device's memory

        Refused while a button in any store is bound to it, so rebind those first. The check takes about 15 s.

        :param macro_id: Macro id
        :type macro_id: int

        :raises ValueError: If a button is still bound to the macro
        :raises NotImplementedError: If function is not supported
        """
        if self.has('onboard_macros'):
            bound = []
            for button in self._dbus_interfaces['onboard'].getButtons():
                for store in range(1, 6):
                    for layer in (0, 1):
                        action = self.get_button_action(int(button), layer, store)
                        if action[0] in (0x03, 0x04) and int.from_bytes(action[2:4], 'big') == macro_id:
                            bound.append("store {0} button 0x{1:02x} layer {2}".format(store, button, layer))
            if bound:
                raise ValueError("macro 0x{0:04x} is still bound to {1}".format(macro_id, ", ".join(bound)))
            self._dbus_interfaces['onboard'].deleteMacro(macro_id)
        else:
            raise NotImplementedError()

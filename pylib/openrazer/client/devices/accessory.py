# SPDX-License-Identifier: GPL-2.0-or-later

from openrazer.client.devices.mice import RazerMouse


class RazerAccessory(RazerMouse):
    """
    Razer accessory client.

    Accessories can expose mouse passthrough capabilities, such as the Mouse Dock Pro
    acting as a receiver for a paired mouse.
    """

# SPDX-License-Identifier: GPL-2.0-or-later

"""Software wheel effect (host-rendered).

Some keyboards do not implement a firmware "wheel" matrix effect.
For those devices we can simulate it by continuously streaming custom
frames (matrix_custom_frame) like Synapse does.
"""

import colorsys
import logging
import math
import threading
import time

from openrazer_daemon.keyboard import KeyboardColour


class WheelEffectThread(threading.Thread):
    def __init__(self, parent, device_number):
        super().__init__()

        self._logger = logging.getLogger(f'razer.device{device_number}.wheelthread')
        self._parent = parent

        self._refresh_rate = 0.040
        self._direction = 1  # 1=right, 2=left

        self._shutdown = False
        self._active = False

        self._rows, self._cols = self._parent._parent.MATRIX_DIMS
        self._keyboard_grid = KeyboardColour(self._rows, self._cols)

        self._phase = 0.0

    @property
    def shutdown(self):
        return self._shutdown

    @shutdown.setter
    def shutdown(self, value):
        self._shutdown = bool(value)

    @property
    def active(self):
        return self._active

    def enable(self, direction, refresh_rate=None):
        if refresh_rate is not None:
            try:
                refresh_rate = float(refresh_rate)
            except (TypeError, ValueError):
                refresh_rate = 0.040
            if refresh_rate <= 0:
                refresh_rate = 0.040
            self._refresh_rate = refresh_rate

        # Direction: 1 right, 2 left
        self._direction = 1 if direction not in (1, 2) else int(direction)
        self._active = True

    def disable(self):
        self._active = False

    def _render_frame(self):
        # Wheel-like rotation: assign hue based on angle around the matrix
        # center, and advance phase over time.
        self._keyboard_grid.reset_rows()

        # Direction: 1=right (clockwise), 2=left (counter-clockwise)
        dir_sign = 1.0 if self._direction == 1 else -1.0
        phase_step = dir_sign * (self._refresh_rate * 0.6)

        cx = (self._cols - 1) / 2.0
        cy = (self._rows - 1) / 2.0
        # Elliptical correction so rotation looks less stretched on wide matrices.
        aspect = (self._cols / max(1.0, float(self._rows)))

        for row in range(self._rows):
            for col in range(self._cols):
                x = (col - cx)
                y = (row - cy) * aspect
                angle = math.atan2(y, x)  # -pi..pi
                # Map angle to 0..1 hue and rotate by phase.
                hue = ((angle / (2.0 * math.pi)) + 0.5 + self._phase) % 1.0
                r_f, g_f, b_f = colorsys.hsv_to_rgb(hue, 1.0, 1.0)
                self._keyboard_grid.set_key_colour(
                    row,
                    col,
                    (int(r_f * 255), int(g_f * 255), int(b_f * 255)),
                )

        payload = self._keyboard_grid.get_total_binary()
        self._parent.set_rgb_matrix(payload)
        self._parent.refresh_keyboard()

        self._phase = (self._phase + phase_step) % 1.0

    def run(self):
        while not self._shutdown:
            if self._active:
                try:
                    self._render_frame()
                except Exception:  # pylint: disable=broad-exception-caught
                    self._logger.exception("WheelEffect frame render failed")
                    # Avoid tight loop if something goes wrong
                    time.sleep(0.25)
                    continue

            time.sleep(self._refresh_rate)


class WheelManager(object):
    """Manages the wheel software effect thread."""

    def __init__(self, parent, device_number):
        self._logger = logging.getLogger(f'razer.device{device_number}.wheelmanager')
        self._parent = parent
        self._parent.register_observer(self)

        self._is_closed = False

        self._wheel_thread = WheelEffectThread(self, device_number)
        self._wheel_thread.start()

    def set_rgb_matrix(self, payload):
        self._parent._set_key_row(payload)

    def refresh_keyboard(self):
        self._parent._set_custom_effect()

    def notify(self, msg):
        if not isinstance(msg, tuple):
            self._logger.warning("Got msg that was not a tuple")
            return

        if msg[0] != 'effect':
            return

        # ('effect', Device, 'effectName', ...params)
        if msg[2] == 'setWheel':
            direction = msg[3] if len(msg) > 3 else 1
            self._wheel_thread.enable(direction)
        else:
            self._wheel_thread.disable()

    def close(self):
        if not self._is_closed:
            self._logger.debug("Closing Wheel Manager")
            self._is_closed = True

            self._wheel_thread.shutdown = True
            self._wheel_thread.join(timeout=2)
            if self._wheel_thread.is_alive():
                self._logger.error("Could not stop WheelEffect thread")

    def __del__(self):
        self.close()

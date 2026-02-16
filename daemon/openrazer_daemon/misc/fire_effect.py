# SPDX-License-Identifier: GPL-2.0-or-later

"""Software fire effect (host-rendered).

Razer Synapse can render some effects on the host by continuously streaming
custom frames (matrix_custom_frame). For devices without a firmware "fire"
matrix effect, we can simulate it in the daemon.

This uses a compact "heat" simulation (Doom-fire-like) and maps intensity to a
fire palette.
"""

import logging
import random
import threading
import time

from openrazer_daemon.keyboard import KeyboardColour


def _clamp_byte(value):
    if value < 0:
        return 0
    if value > 255:
        return 255
    return int(value)


class _FirePalettes(object):
    """Palette helpers for mapping an intensity 0..255 to RGB."""

    # Warm palette (no green/blue hues).
    WARM = [
        (0, (0, 0, 0)),
        (40, (48, 0, 0)),
        (90, (180, 0, 0)),
        (140, (255, 40, 0)),
        (190, (255, 140, 0)),
        (235, (255, 220, 0)),
        (255, (255, 255, 120)),
    ]

    # Blue gas flame style (deep navy -> vivid blue -> cyan -> white).
    BLUE = [
        (0, (0, 0, 0)),
        (40, (0, 0, 40)),
        (90, (0, 0, 150)),
        (140, (0, 90, 255)),
        (190, (90, 230, 255)),
        (235, (230, 255, 255)),
        (255, (255, 255, 255)),
    ]

    # Spectral/magical green (deep green -> neon green -> pale mint -> white).
    SPECTRAL = [
        (0, (0, 0, 0)),
        (50, (0, 25, 0)),
        (100, (0, 90, 25)),
        (150, (0, 220, 70)),
        (200, (150, 255, 150)),
        (235, (230, 255, 230)),
        (255, (255, 255, 255)),
    ]

    # Magic fire (white <-> purple). Dark purple -> vivid purple -> lavender -> white.
    MAGIC = [
        (0, (0, 0, 0)),
        (40, (18, 0, 35)),
        (90, (60, 0, 110)),
        (140, (130, 0, 210)),
        (190, (210, 90, 255)),
        (230, (245, 225, 255)),
        (255, (255, 255, 255)),
    ]

    @staticmethod
    def rgb_from_stops(stops, intensity):
        intensity = _clamp_byte(intensity)
        if not stops:
            return (0, 0, 0)

        prev_i, prev_c = stops[0]
        for next_i, next_c in stops[1:]:
            if intensity <= next_i:
                if next_i == prev_i:
                    return prev_c
                t = (intensity - prev_i) / float(next_i - prev_i)
                r = int(prev_c[0] + (next_c[0] - prev_c[0]) * t)
                g = int(prev_c[1] + (next_c[1] - prev_c[1]) * t)
                b = int(prev_c[2] + (next_c[2] - prev_c[2]) * t)
                return (_clamp_byte(r), _clamp_byte(g), _clamp_byte(b))
            prev_i, prev_c = next_i, next_c

        return stops[-1][1]

    @staticmethod
    def stops_from_rgb_list(rgb_list):
        """Build evenly-spaced intensity stops from [(r,g,b), ...]."""
        if not rgb_list:
            return _FirePalettes.WARM
        if len(rgb_list) == 1:
            rgb_list = [(0, 0, 0), rgb_list[0], (255, 255, 255)]

        stops = []
        n = len(rgb_list)
        for i, (r, g, b) in enumerate(rgb_list):
            intensity = int(round((255.0 * i) / float(n - 1))) if n > 1 else 255
            stops.append((intensity, (_clamp_byte(r), _clamp_byte(g), _clamp_byte(b))))

        # Ensure it starts at 0 and ends at 255.
        if stops[0][0] != 0:
            stops.insert(0, (0, (0, 0, 0)))
        if stops[-1][0] != 255:
            stops.append((255, stops[-1][1]))
        return stops

    @staticmethod
    def stops_for_variant(variant: int):
        # 0 warm, 1 blue, 2 spectral, 3 magic
        if variant == 1:
            return _FirePalettes.BLUE
        if variant == 2:
            return _FirePalettes.SPECTRAL
        if variant == 3:
            return _FirePalettes.MAGIC
        return _FirePalettes.WARM


class FireEffectThread(threading.Thread):
    def __init__(self, parent, device_number):
        super().__init__()

        self._logger = logging.getLogger(f'razer.device{device_number}.firethread')
        self._parent = parent

        self._refresh_rate = 0.040
        self._speed = 3  # 1..4

        self._shutdown = False
        self._active = False

        self._rows, self._cols = self._parent._parent.MATRIX_DIMS
        self._keyboard_grid = KeyboardColour(self._rows, self._cols)

        # Heat buffer (rows x cols), values 0..255
        self._heat = [[0 for _ in range(self._cols)] for _ in range(self._rows)]

        # Palette stops for intensity->RGB mapping
        self._palette_stops = list(_FirePalettes.WARM)

        # Tracks which built-in variant is active (0 warm, 1 blue, 2 spectral, 3 magic).
        # Custom palette uses -1.
        self._variant_id = 0

        # Synchronize enable/disable against in-flight frame renders so that
        # switching to a non-custom effect (e.g. static) doesn't get overwritten
        # by one last custom-frame update.
        self._render_lock = threading.Lock()

    @property
    def shutdown(self):
        return self._shutdown

    @shutdown.setter
    def shutdown(self, value):
        self._shutdown = bool(value)

    @property
    def active(self):
        return self._active

    def enable(self, speed=None, variant=None, palette=None):
        with self._render_lock:
            if speed is None:
                speed = self._speed

            try:
                speed = int(speed)
            except (TypeError, ValueError):
                speed = 3

            if speed not in (1, 2, 3, 4):
                speed = 3

            self._speed = speed

            if variant is not None:
                try:
                    variant_i = int(variant)
                except (TypeError, ValueError):
                    variant_i = 0
                self._palette_stops = list(_FirePalettes.stops_for_variant(variant_i))
                self._variant_id = variant_i

            if palette is not None:
                rgb_list = []
                try:
                    if isinstance(palette, (bytes, bytearray)):
                        palette_bytes = bytes(palette)
                    else:
                        # e.g. dbus.ByteArray
                        palette_bytes = bytes(bytearray(palette))

                    # Expect multiples of 3 bytes: R,G,B repeating.
                    if len(palette_bytes) >= 3 and (len(palette_bytes) % 3) == 0:
                        for i in range(0, len(palette_bytes), 3):
                            rgb_list.append((palette_bytes[i], palette_bytes[i + 1], palette_bytes[i + 2]))
                except Exception:  # pylint: disable=broad-exception-caught
                    rgb_list = []

                if rgb_list:
                    self._palette_stops = list(_FirePalettes.stops_from_rgb_list(rgb_list))
                    self._variant_id = -1

            # Higher speed = faster updates
            self._refresh_rate = {1: 0.060, 2: 0.045, 3: 0.035, 4: 0.025}[self._speed]
            self._active = True

    def disable(self):
        self._active = False
        # Wait for any in-flight frame render to complete.
        with self._render_lock:
            return

    def _step_simulation(self):
        # Parameterize for small matrices.
        # Cooling grows with speed; sparks also become more frequent.
        # NOTE: These are tuned to fill more of the keyboard on small (e.g. 6-row) matrices.
        cooling = {1: 12, 2: 16, 3: 19, 4: 22}[self._speed]
        spark_prob = {1: 0.16, 2: 0.20, 3: 0.24, 4: 0.28}[self._speed]

        new_heat = [[0 for _ in range(self._cols)] for _ in range(self._rows)]

        bottom = self._rows - 1
        ignite_rows = min(3, self._rows)

        # Ignition zone: bottom N rows get fresh heat (so the effect is not limited to 1-2 rows).
        for y in range(bottom, bottom - ignite_rows, -1):
            row_boost = (bottom - y)  # 0 for bottom row, 1 for row above, ...
            # Slightly reduce probability/intensity as we go up.
            p = spark_prob * (1.0 - 0.25 * row_boost)
            p = max(0.05, p)
            for x in range(self._cols):
                val = self._heat[y][x]
                val = max(0, val - random.randint(0, max(1, cooling // 3)))

                if random.random() < p:
                    val = 255
                elif random.random() < (p * 0.35):
                    val = max(val, 220)
                elif random.random() < (p * 0.45):
                    val = max(val, 160)

                new_heat[y][x] = val

        # Propagate upwards with diffusion. Use below row(s) and diagonals to widen the flame.
        for y in range(bottom - ignite_rows, -1, -1):
            y1 = y + 1
            y2 = min(bottom, y + 2)

            # Reduce cooling higher up so it doesn't die out immediately on small matrices.
            height_t = (bottom - y) / max(1.0, float(bottom))  # 0 at bottom, ~1 at top
            local_cooling = int(cooling * (0.55 + 0.35 * height_t))

            for x in range(self._cols):
                # Weighted average from below and two-below rows.
                xl = (x - 1) % self._cols
                xr = (x + 1) % self._cols

                base = (
                    new_heat[y1][x] * 5 +
                    new_heat[y1][xl] * 2 +
                    new_heat[y1][xr] * 2 +
                    new_heat[y2][x] * 2
                ) // 11

                # Random drift (small), mostly to avoid vertical banding.
                drift = random.choice((-1, 0, 1))
                base = (base + new_heat[y1][(x + drift) % self._cols]) // 2

                dec = random.randint(0, max(1, local_cooling))
                new_heat[y][x] = max(0, int(base) - dec)

        self._heat = new_heat

    def _render_frame(self):
        self._keyboard_grid.reset_rows()

        for row in range(self._rows):
            for col in range(self._cols):
                intensity = self._heat[row][col]

                # Make upper rows darker so the gradient is clearly visible.
                # row=0 is top, row=bottom is brightest.
                if self._rows > 1:
                    if self._variant_id in (1, 2, 3):
                        # Stronger gradient for blue/spectral/magic variants.
                        row_factor = 0.35 + 0.65 * (row / float(self._rows - 1))
                    else:
                        row_factor = 0.60 + 0.40 * (row / float(self._rows - 1))
                else:
                    row_factor = 1.0

                # Gentle gamma to reduce harsh low-intensity colors.
                scaled = int((intensity * row_factor) ** 0.95)

                self._keyboard_grid.set_key_colour(
                    row,
                    col,
                    _FirePalettes.rgb_from_stops(self._palette_stops, scaled),
                )

        payload = self._keyboard_grid.get_total_binary()
        self._parent.set_rgb_matrix(payload)
        self._parent.refresh_keyboard()

    def run(self):
        while not self._shutdown:
            if self._active:
                try:
                    with self._render_lock:
                        # Re-check inside the lock so disable() can't race a frame.
                        if self._active:
                            self._step_simulation()
                            self._render_frame()
                except Exception:  # pylint: disable=broad-exception-caught
                    self._logger.exception("FireEffect frame render failed")
                    time.sleep(0.25)
                    continue

            time.sleep(self._refresh_rate)


class FireManager(object):
    """Manages the fire software effect thread."""

    def __init__(self, parent, device_number):
        self._logger = logging.getLogger(f'razer.device{device_number}.firemanager')
        self._parent = parent
        self._parent.register_observer(self)

        self._is_closed = False

        self._fire_thread = FireEffectThread(self, device_number)
        self._fire_thread.start()

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
        effect_name = msg[2]

        if effect_name == 'setFire':
            speed = msg[3] if len(msg) > 3 else 3
            # Reset to default (warm) palette when switching back to plain fire.
            self._fire_thread.enable(speed=speed, variant=0)
            return

        if effect_name == 'setFireVariant':
            speed = msg[3] if len(msg) > 3 else 3
            variant = msg[4] if len(msg) > 4 else 0
            self._fire_thread.enable(speed=speed, variant=variant)
            return

        if effect_name == 'setFirePalette':
            speed = msg[3] if len(msg) > 3 else 3
            palette = msg[4] if len(msg) > 4 else None
            self._fire_thread.enable(speed=speed, palette=palette)
            return

        self._fire_thread.disable()

    def close(self):
        if not self._is_closed:
            self._logger.debug("Closing Fire Manager")
            self._is_closed = True

            self._fire_thread.shutdown = True
            self._fire_thread.join(timeout=2)
            if self._fire_thread.is_alive():
                self._logger.error("Could not stop FireEffect thread")

    def __del__(self):
        self.close()

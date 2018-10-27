import sys
import time

import openrazer.client

device_manager = openrazer.client.DeviceManager()
keyboard = None

for device in device_manager.devices:
    if device.type == 'keyboard':
        keyboard = device
        break
else:
    print("Could not find suitable keyboard", file=sys.stderr)
    sys.exit(1)

if not keyboard.has('lighting_led_matrix'):
    print("Keyboard doesn't have LED matrix", file=sys.stderr)
    sys.exit(1)

ROWS = 6
COLS = 22
COLOUR = (0, 255, 0)

keyboard.fx.none()

last_row = 5
last_col = 0

while last_row >= 0 and last_col >= 0:
    for row in range(0, ROWS):
        if row % 2 == 0:
            col_range = range(0, COLS)
        else:
            col_range = range(COLS - 1, -1, -1)

        for col in col_range:
            if not (row == 0 and col in (18, 19, 20, 21)) and not (row == 5 and col in (4, 5, 6, 8, 9, 10)):  # Skip big dead area
                keyboard.fx.advanced.matrix.reset()
                keyboard.fx.advanced.matrix[row, col] = COLOUR
                keyboard.fx.advanced.draw_fb_or()
                # keyboard.fx.advanced.draw()
                time.sleep(0.05)

            if row == last_row and col == last_col:
                if row % 2 == 0:
                    last_col -= 1
                else:
                    last_col += 1
                keyboard.fx.advanced.matrix.to_framebuffer_or()

        if row == last_row:
            if row % 2 == 0 and last_col == 0:
                last_row -= 1
            if row % 2 != 0 and last_col == 21:
                last_row -= 1

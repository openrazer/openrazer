"""
Example of changing the colour of all Razer devices based on the CPU
temperature
"""
import colorsys
import time

import openrazer.client

# These settings are nice if your CPU temperature is a bit less than 30°C
# at idle and a bit more than 60°C at full load, tune them for your needs
tGreen = 30
tRed = 60

tRange = 3 * (tRed - tGreen)
colour0 = tRed / tRange
tRange = 1000 * tRange

devman = openrazer.client.DeviceManager()
devman.sync_effects = False
devices = devman.devices

if not devices:
    print("No devices connected")
    exit(1)

lastTemp = 0

while True:
    # For my computer it's hwmon2/temp1_input but it may be different for you
    # cat /sys/class/hwmon/hwmonX/tempY_input can help you (with X between 0
    # and 2 and Y between 0 and 12, the result is in millidegrees).
    # you will need lm-sensors for this to work.
    with open("/sys/class/hwmon/hwmon2/temp1_input", "r") as f:
        temp = f.readline()

    shortTemp = temp[:2]

    # Now we can change the colour each time temperature changes
    if shortTemp != lastTemp:
        # We convert the temperature (millidegrees) to the hue
        # (HSV, hue between 0 and 1)
        hue = (colour0 - float(temp) / tRange) % 1

        # The colours list RGB with R,G,B between 0 and 1
        colour = colorsys.hsv_to_rgb(hue, 1, 1)

        # Needs to be an integer (0-255, e.g. 1 == 255)
        colour = list(map(lambda x: (int(255 * x)), colour))

        # Set effect on device
        for device in devices:
            # A device may have multiple zones
            if device.has("lighting_static"):
                device.fx.static(colour[0], colour[1], colour[2])
            if device.has("lighting_backlight"):
                device.fx.misc.backlight.static(colour[0], colour[1], colour[2])
            if device.has("lighting_logo_static"):
                device.fx.misc.logo.static(colour[0], colour[1], colour[2])
            if device.has("lighting_scroll_static"):
                device.fx.misc.scroll_wheel.static(colour[0], colour[1], colour[2])
            if device.has("lighting_left_static"):
                device.fx.misc.left.static(colour[0], colour[1], colour[2])
            if device.has("lighting_right_static"):
                device.fx.misc.right.static(colour[0], colour[1], colour[2])

        lastTemp = shortTemp

    time.sleep(1)

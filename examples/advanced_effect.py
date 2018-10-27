import colorsys
import random

from openrazer.client import DeviceManager
from openrazer.client import constants as razer_constants

# Create a DeviceManager. This is used to get specific devices
device_manager = DeviceManager()


print("Found {} Razer devices".format(len(device_manager.devices)))
print()

# Disable daemon effect syncing.
# Without this, the daemon will try to set the lighting effect to every device.
device_manager.sync_effects = False

# List of effect I've chosen to make an example for
effects = [
    'breath_random',
    'breath_single',
    'breath_dual',
    'breath_triple',
    'reactive',
    'spectrum',
    'static',
    'wave',
]

# Helper function to generate interesting colors


def random_color():
    rgb = colorsys.hsv_to_rgb(random.uniform(0, 1), random.uniform(0.5, 1), 1)
    return tuple(map(lambda x: int(256 * x), rgb))


# Iterate over each device and set a random effect that it supports.
for device in device_manager.devices:
    # Check which effect this device supports.
    device_effects = [effect for effect in effects if device.fx.has(effect)]
    # print("{} supports {}".format(device.name, device_effects))

    if len(device_effects) == 0:
        print("Device {} doesn't support any of the effects".format(device.name))
        continue
    effect = random.choice(device_effects)
    print("Setting {} to effect {}".format(device.name, effect))

    # Ad an example for each effect
    if effect == 'breath_random':
        device.fx.breath_random()

    elif effect == 'breath_single':
        color = random_color()
        device.fx.breath_single(color[0], color[1], color[2])

    elif effect == 'breath_dual':
        color = random_color()
        color2 = random_color()
        device.fx.breath_dual(color[0], color[1], color[2],
                              color2[0], color2[1], color2[2])

    elif effect == 'breath_triple':
        color = random_color()
        color2 = random_color()
        color3 = random_color()
        device.fx.breath_triple(color[0], color[1], color[2],
                                color2[0], color2[1], color2[2],
                                color3[0], color3[1], color3[2])

    elif effect == 'reactive':
        color = random_color()
        times = [razer_constants.REACTIVE_500MS, razer_constants.REACTIVE_1000MS,
                 razer_constants.REACTIVE_1500MS, razer_constants.REACTIVE_2000MS]
        device.fx.reactive(color[0], color[1], color[2], random.choice(times))

    elif effect == 'spectrum':
        device.fx.spectrum()

    elif effect == 'static':
        color = random_color()
        device.fx.static(*color)

    elif effect == 'wave':
        directions = [razer_constants.WAVE_LEFT, razer_constants.WAVE_RIGHT]
        device.fx.wave(random.choice(directions))
